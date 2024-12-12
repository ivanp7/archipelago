/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Executable entry function.
 */

#include "archi/exe/main.fun.h"
#include "archi/exe/main.def.h"
#include "archi/exe/args.typ.h"
#include "archi/app.h"
#include "archi/fsm.h"
#include "archi/plugin/threads/vtable.var.h"
#include "archi/util/error.def.h"
#include "archi/util/flexible.def.h"
#include "archi/util/list.fun.h"
#include "archi/util/os/shm.fun.h"
#include "archi/util/os/signal.fun.h"
#include "archi/util/os/signal.typ.h"
#include "archi/util/os/signal.def.h"
#include "archi/util/print.fun.h"
#include "archi/util/print.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), strlen()
#include <stdio.h> // for printf()
#include <stdatomic.h> // for atomic_flag, ATOMIC_FLAG_INIT

/*****************************************************************************/

#define ARCHI_PELAGO_LOGO "\
  ⡏ ⢀⣀ ⡀⣀ ⢀⣀ ⣇⡀ ⠄ ⣀⡀ ⢀⡀ ⡇ ⢀⣀ ⢀⡀ ⢀⡀ ⢹  \n\
  ⣇ ⠣⠼ ⠏  ⠣⠤ ⠇⠸ ⠇ ⡧⠜ ⠣⠭ ⠣ ⠣⠼ ⣑⡺ ⠣⠜ ⣸  \n\
"

#define M "archi_main()" // module name for logging

/*****************************************************************************/

struct archi_app_signal_handler_with_lock {
    archi_signal_handler_t signal_handler;
    atomic_flag spinlock;
};

struct archi_app_context {
    // Finite state machine
    archi_state_t entry_state;
    archi_state_transition_t state_transition;

    // Signal management
    struct archi_signal_management_context *signal_management;
    struct archi_app_signal_handler_with_lock signal_handler_with_lock;
};

/*****************************************************************************/

static
ARCHI_PLUGIN_SET_FUNC(archi_app_vtable_set)
{
    if ((context == NULL) || (slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    struct archi_app_context *app_context = context;

    if (strcmp(slot, ARCHI_APP_CONTEXT_PORT_ENTRY_STATE_FUNC) == 0) // entry state function
    {
        if ((value->ptr == NULL) || (value->size != sizeof(archi_state_function_t)) ||
                (value->num_of != 1) || (value->type != ARCHI_VALUE_POINTER))
            return ARCHI_ERROR_CONFIG;

        archi_state_function_t *fptr = value->ptr;
        app_context->entry_state.function = *fptr;
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_PORT_ENTRY_STATE_DATA) == 0) // entry state data
        app_context->entry_state.data = value->ptr;
    else if (strcmp(slot, ARCHI_APP_CONTEXT_PORT_STATE_TRANS_FUNC) == 0) // state transition function
    {
        if ((value->ptr == NULL) || (value->size != sizeof(archi_state_transition_function_t)) ||
                (value->num_of != 1) || (value->type != ARCHI_VALUE_POINTER))
            return ARCHI_ERROR_CONFIG;

        archi_state_transition_function_t *fptr = value->ptr;
        app_context->state_transition.function = *fptr;
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_PORT_STATE_TRANS_DATA) == 0) // state transition data
        app_context->state_transition.data = value->ptr;
    else if (strcmp(slot, ARCHI_APP_CONTEXT_PORT_SIGNAL_HANDLER_FUNCTION) == 0) // signal handler function
    {
        if ((value->ptr == NULL) || (value->size != sizeof(archi_signal_handler_func_t)) ||
                (value->num_of != 1) || (value->type != ARCHI_VALUE_POINTER))
            return ARCHI_ERROR_CONFIG;

        archi_signal_handler_func_t *fptr = value->ptr;

        while (atomic_flag_test_and_set_explicit(&app_context->signal_handler_with_lock.spinlock, memory_order_acquire));
        app_context->signal_handler_with_lock.signal_handler.function = *fptr;
        atomic_flag_clear_explicit(&app_context->signal_handler_with_lock.spinlock, memory_order_release);
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_PORT_SIGNAL_HANDLER_DATA) == 0) // signal handler data
    {
        while (atomic_flag_test_and_set_explicit(&app_context->signal_handler_with_lock.spinlock, memory_order_acquire));
        app_context->signal_handler_with_lock.signal_handler.data = value->ptr;
        atomic_flag_clear_explicit(&app_context->signal_handler_with_lock.spinlock, memory_order_release);
    }
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

static
ARCHI_PLUGIN_GET_FUNC(archi_app_vtable_get)
{
    if ((context == NULL) || (slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    struct archi_app_context *app_context = context;

    if (strcmp(slot, ARCHI_APP_CONTEXT_PORT_SIGNAL_SET) == 0) // signal flags
    {
        if (app_context->signal_management != NULL)
        {
            archi_signal_flags_t *flags;
            archi_signal_management_thread_get_properties(
                    app_context->signal_management, &flags, NULL);

            value->ptr = flags;
            value->size = ARCHI_SIGNAL_FLAGS_SIZEOF;
            value->num_of = 1;
            value->type = ARCHI_VALUE_DATA;
        }
        else
            *value = (archi_value_t){0};
    }

    return 0;
}

static
const archi_plugin_vtable_t archi_app = {
    .format = {.magic = ARCHI_API_MAGIC, .version = ARCHI_API_VERSION},
    .info = {.description = "Application interface"},
    .func = {.set_fn = archi_app_vtable_set, .get_fn = archi_app_vtable_get},
};

static
const archi_plugin_vtable_t *const archi_builtin_vtables[] = {
    &archi_app,
    &archi_threads,
};

/*****************************************************************************/

static
ARCHI_SIGNAL_HANDLER_FUNC(archi_main_signal_handler_func)
{
    if (data == NULL)
        return true;

    archi_signal_handler_t signal_handler = {0};
    {
        struct archi_app_signal_handler_with_lock *ptr = data;
        while (atomic_flag_test_and_set_explicit(&ptr->spinlock, memory_order_acquire));
        signal_handler = ptr->signal_handler;
        atomic_flag_clear_explicit(&ptr->spinlock, memory_order_release);
    }

    if (signal_handler.function != NULL)
        return signal_handler.function(signo, siginfo, signals, signal_handler.data);
    else
        return true;
}

/*****************************************************************************/

static
archi_status_t
archi_init_app_builtins(
        archi_application_t *app,
        struct archi_app_context *app_context)
{
    archi_log_debug(M, "Adding built-in virtual tables to the application...");

    size_t num_app_vtable_nodes;
    archi_app_vtable_instance_t *app_vtable_nodes;
    {
        num_app_vtable_nodes = sizeof(archi_builtin_vtables) / sizeof(archi_builtin_vtables[0]);

        // Allocate nodes
        app_vtable_nodes = malloc(sizeof(*app_vtable_nodes) * num_app_vtable_nodes);
        if (app_vtable_nodes == NULL)
        {
            archi_log_error(M, "Couldn't allocate application virtual table nodes.");
            return ARCHI_ERROR_ALLOC;
        }

        // Initialize nodes
        for (size_t i = 0; i < num_app_vtable_nodes; i++)
            app_vtable_nodes[i] = (archi_app_vtable_instance_t){.vtable = archi_builtin_vtables[i]};

        // Allocate node names
        for (size_t i = 0; i < num_app_vtable_nodes; i++)
        {
            app_vtable_nodes[i].base.name = archi_application_vtable_alias_alloc(
                    NULL, archi_builtin_vtables[i]->info.name);
            if (app_vtable_nodes[i].base.name == NULL)
            {
                archi_log_error(M, "Couldn't allocate application virtual table node name.");

                for (size_t j = 0; j < i; j++)
                    free(app_vtable_nodes[j].base.name);

                return ARCHI_ERROR_ALLOC;
            }
        }

        // Add nodes to the list
        for (size_t i = 0; i < num_app_vtable_nodes; i++)
            archi_list_insert_node(&app->vtables, (archi_list_node_t*)&app_vtable_nodes[i], NULL, NULL, false); // append
    }

    archi_log_debug(M, "Adding built-in contexts to the application...");

    {
        // Allocate node
        archi_app_context_instance_t *app_context_node = malloc(sizeof(*app_context_node));
        if (app_context_node == NULL)
        {
            archi_log_error(M, "Couldn't allocate the application context node.");
            return ARCHI_ERROR_ALLOC;
        }

        *app_context_node = (archi_app_context_instance_t){
            .context = app_context, .vtable_node = app_vtable_nodes};

        // Allocate node name
        size_t name_len = strlen(ARCHI_APP_CONTEXT_ALIAS);
        if (app_context_node->base.name == NULL)
        {
            archi_log_error(M, "Couldn't allocate the application context node name.");

            free(app_context_node);
            return ARCHI_ERROR_ALLOC;
        }

        memcpy(app_context_node->base.name, ARCHI_APP_CONTEXT_ALIAS, name_len + 1);

        // Add node to the list
        archi_list_insert_node(&app->contexts, (archi_list_node_t*)app_context_node, NULL, NULL, false); // append
    }

    return 0;
}

/*****************************************************************************/

struct archi_app_main_local_variables {
    void **shmaddr; // shared memory address
    const archi_app_configuration_t *app_config; // application configuration

    // Application execution mode
    struct archi_app_context app_context; // application context

    // Plugin probe mode
    archi_app_configuration_t probe_config; // plugin probe mode configuration
    archi_app_config_plugin_list_node_t probe_config_plugin_node; // configuration node for the plugin probe mode
};

static
archi_status_t
archi_prepare_plugin_help_mode(
        archi_application_t *app,
        const archi_cmdline_args_t *args,
        struct archi_app_main_local_variables *l)
{
    archi_log_debug(M, "Preparing application configuration for the plugin probe mode...");

    l->probe_config_plugin_node = (archi_app_config_plugin_list_node_t){
        .base.name = "plugin", // any valid name will do here
            .pathname = args->probe_mode.pathname,
            .num_vtables = 1,
            .vtable_symbols = (char**)&args->probe_mode.vtable_symbol,
    };

    l->probe_config = (archi_app_configuration_t){
        .plugins = {.head = (archi_list_node_t*)&l->probe_config_plugin_node,
            .tail = (archi_list_node_t*)&l->probe_config_plugin_node},
    };

    l->shmaddr = NULL;
    l->app_config = &l->probe_config;

    *app = (archi_application_t){0};
    return 0;
}

static
archi_status_t
archi_prepare_execution_mode(
        archi_application_t *app,
        const archi_cmdline_args_t *args,
        struct archi_app_main_local_variables *l)
{
    // Initialize the application context
    l->app_context = (struct archi_app_context){.signal_handler_with_lock.spinlock = ATOMIC_FLAG_INIT};

    // Attach to shared memory
    archi_log_debug(M, "Attaching to shared memory...");

    l->shmaddr = archi_shared_memory_attach(args->exec_mode.pathname, args->exec_mode.proj_id, false);
    if (l->shmaddr == NULL)
    {
        archi_log_error(M, "Couldn't attach to shared memory at pathname '%s', project id %i.",
                args->exec_mode.pathname, args->exec_mode.proj_id);

        return ARCHI_ERROR_ATTACH;
    }

    l->app_config = l->shmaddr[ARCHI_SHM_APP_CONFIG_INDEX];

    // Initialize signal management
    {
        const archi_signal_watch_set_t *signal_watch_set = l->shmaddr[ARCHI_SHM_SIGNAL_WATCH_SET_INDEX];
        if (signal_watch_set != NULL)
        {
            archi_log_debug(M, "Starting signal management thread...");

            l->app_context.signal_management = archi_signal_management_thread_start(signal_watch_set,
                    (archi_signal_handler_t){.function = archi_main_signal_handler_func,
                    .data = &l->app_context.signal_handler_with_lock.signal_handler});
            if (l->app_context.signal_management == NULL)
            {
                archi_log_error(M, "Couldn't start signal management thread.");

                return ARCHI_ERROR_SIGNAL;
            }
        }
    }

    // Initialize the application instance with builtins
    *app = (archi_application_t){0};
    return archi_init_app_builtins(app, &l->app_context);
}

static
void
archi_main_cleanup(
        struct archi_app_main_local_variables *l)
{
    if (l->app_context.signal_management != NULL)
    {
        archi_log_debug(M, "Stopping signal management thread...");

        archi_signal_management_thread_stop(l->app_context.signal_management);
    }

    if (l->shmaddr != NULL)
    {
        archi_log_debug(M, "Detaching from shared memory...");

        archi_shared_memory_detach(l->shmaddr);
    }
}

/*****************************************************************************/

static
archi_status_t
archi_display_plugin_help(
        archi_application_t *app,
        const char *topic)
{
    if (app->vtables.head == NULL)
        return ARCHI_ERROR_MISUSE;

    const archi_plugin_vtable_t *vtable;
    {
        archi_app_vtable_instance_t *vtable_node = (archi_app_vtable_instance_t*)app->vtables.head;

        vtable = vtable_node->vtable;
        if (vtable == NULL)
            return ARCHI_ERROR_MISUSE;
    }

    printf("Name: %s\n", vtable->info.name);

    if (vtable->info.description != NULL)
        printf("Description: %s\n", vtable->info.description);
    else
        printf("Description is not provided\n");

    printf("Functions:\n");
#define PRINTF_FUNC(name, field) do { \
    printf("  " name ": %s\n", (vtable->field != NULL) ? "provided" : "unavailable"); \
    } while (0)

    PRINTF_FUNC("help",         info.help_fn);
    PRINTF_FUNC("initialize",   func.init_fn);
    PRINTF_FUNC("finalize",     func.final_fn);
    PRINTF_FUNC("setter",       func.set_fn);
    PRINTF_FUNC("getter",       func.get_fn);
    PRINTF_FUNC("action",       func.act_fn);

#undef PRINTF_FUNC

    if (topic != NULL)
    {
        if (vtable->info.help_fn == NULL)
        {
            archi_log_error(M, "Plugin help function is not available.");
            return ARCHI_ERROR_FUNCTION;
        }

        printf("\nHelp on topic '%s':\n", topic);
        return vtable->info.help_fn(topic);
    }
    else
        return 0;
}

/*****************************************************************************/

int
archi_main(
        archi_application_t *app,
        const archi_cmdline_args_t *args)
{
    if ((app == NULL) || (args == NULL))
        return ARCHI_EXIT_CODE(ARCHI_ERROR_MISUSE);

    struct archi_app_main_local_variables l; // local variables
    archi_status_t code; // status of operations

    //////////////////////
    // Application logo //
    //////////////////////

    {
        archi_print(ARCHI_COLOR_RESET "\n");
        if (!args->no_logo)
            archi_print(ARCHI_COLOR_FG_BRI_WHITE "%s" ARCHI_COLOR_RESET "\n\n", ARCHI_PELAGO_LOGO);
    }

    //////////////////////
    // Preparation step //
    //////////////////////

    {
        if (args->probe_mode.pathname != NULL) // plugin probe mode
            code = archi_prepare_plugin_help_mode(app, args, &l);
        else // application execution mode
            code = archi_prepare_execution_mode(app, args, &l);

        if (code != 0)
            goto finalization;
    }

    /////////////////////////
    // Initialization step //
    /////////////////////////

    {
        archi_log_info(M, "Initializing the application...");

        /*****************************************************/
        code = archi_application_initialize(app, l.app_config);
        /*****************************************************/

        // Exit on error
        if (code != 0)
        {
            archi_log_error(M, "Couldn't initialize the application.");
            goto finalization;
        }
    }

    ////////////////////
    // Execution step //
    ////////////////////

    if (args->probe_mode.pathname != NULL) // plugin probe mode
    {
        /*****************************************************************/
        code = archi_display_plugin_help(app, args->probe_mode.help_topic);
        /*****************************************************************/

        if (code != 0)
            archi_log_error(M, "Couldn't provide application plugin help.");
    }
    else // application execution mode
    {
        // Run the application
        if (l.app_context.entry_state.function == NULL)
        {
            archi_log_error(M, "Application entry state is null.");

            code = ARCHI_ERROR_CONFIG;
            goto finalization;
        }

        archi_log_info(M, "Running the application...");

        /*******************************************************************************************/
        /*******************************************************************************************/
        code = archi_finite_state_machine(l.app_context.entry_state, l.app_context.state_transition);
        /*******************************************************************************************/
        /*******************************************************************************************/
    }

    ///////////////////////
    // Finalization step //
    ///////////////////////

finalization:
    {
        archi_log_info(M, "Finalizing the application...");

        /******************************/
        archi_application_finalize(app);
        /******************************/
    }

    /////////////
    // Cleanup //
    /////////////

    archi_main_cleanup(&l);

    // Return status code transformed to exit code
    return ARCHI_EXIT_CODE(code);
}

