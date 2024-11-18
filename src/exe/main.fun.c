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
#include "archi/util/error.def.h"
#include "archi/util/os.fun.h"
#include "archi/util/print.fun.h"
#include "archi/util/print.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), strlen()
#include <stdio.h> // for printf()

/*****************************************************************************/

#define ARCHI_PELAGO_LOGO "\
  ⡏ ⢀⣀ ⡀⣀ ⢀⣀ ⣇⡀ ⠄ ⣀⡀ ⢀⡀ ⡇ ⢀⣀ ⢀⡀ ⢀⡀ ⢹  \n\
  ⣇ ⠣⠼ ⠏  ⠣⠤ ⠇⠸ ⠇ ⡧⠜ ⠣⠭ ⠣ ⠣⠼ ⣑⡺ ⠣⠜ ⣸  \n\
"

#define M "archi_main()" // module name for logging

/*****************************************************************************/

struct archi_app_context {
    archi_state_t entry_state;
    archi_state_transition_t state_transition;
};

static ARCHI_PLUGIN_SET_FUNC(archi_app_vtable_set)
{
    if ((context == NULL) || (port == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    struct archi_app_context *app_context = context;

    if (strcmp(port, ARCHI_APP_CONTEXT_PORT_ENTRY_STATE_FUNC) == 0) // entry state function
    {
        if ((value->ptr == NULL) || (value->size != sizeof(archi_state_function_t)) ||
                (value->num_of != 1) || (value->type != ARCHI_VALUE_POINTER))
            return ARCHI_ERROR_CONFIG;

        archi_state_function_t *fptr = value->ptr;
        app_context->entry_state.function = *fptr;
    }
    else if (strcmp(port, ARCHI_APP_CONTEXT_PORT_ENTRY_STATE_DATA) == 0) // entry state data
        app_context->entry_state.data = value->ptr;
    else if (strcmp(port, ARCHI_APP_CONTEXT_PORT_STATE_TRANS_FUNC) == 0) // state transition function
    {
        if ((value->ptr == NULL) || (value->size != sizeof(archi_state_transition_function_t)) ||
                (value->num_of != 1) || (value->type != ARCHI_VALUE_POINTER))
            return ARCHI_ERROR_CONFIG;

        archi_state_transition_function_t *fptr = value->ptr;
        app_context->state_transition.function = *fptr;
    }
    else if (strcmp(port, ARCHI_APP_CONTEXT_PORT_STATE_TRANS_DATA) == 0) // state transition data
        app_context->state_transition.data = value->ptr;
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

/*****************************************************************************/

static
archi_status_t
archi_main_prepare_app(
        archi_application_t *app,

        archi_plugin_vtable_t *app_vtable,
        struct archi_app_context *app_context)
{
    archi_log_debug(M, "Initializing application virtual table and context...");

    archi_app_vtable_instance_t *app_vtable_node;
    {
        // Allocate node
        app_vtable_node = malloc(sizeof(*app_vtable_node));
        if (app_vtable_node == NULL)
        {
            archi_log_error(M, "Couldn't allocate the application vtable node.");
            return ARCHI_ERROR_ALLOC;
        }

        *app_vtable_node = (archi_app_vtable_instance_t){.vtable = app_vtable};

        // Allocate node name
        app_vtable_node->base.name = malloc(1);
        if (app_vtable_node->base.name == NULL)
        {
            free(app_vtable_node);

            archi_log_error(M, "Couldn't allocate the application vtable node name.");
            return ARCHI_ERROR_ALLOC;
        }

        app_vtable_node->base.name[0] = '\0'; // application vtable name is ""
    }

    archi_app_context_instance_t *app_context_node;
    {
        // Allocate node
        app_context_node = malloc(sizeof(*app_context_node));
        if (app_context_node == NULL)
        {
            archi_application_forget_vtable(app_vtable_node);

            archi_log_error(M, "Couldn't allocate the application context node.");
            return ARCHI_ERROR_ALLOC;
        }

        *app_context_node = (archi_app_context_instance_t){
            .context = app_context, .vtable_node = app_vtable_node};

        // Allocate node name
        size_t name_size = strlen(ARCHI_APP_CONTEXT_ALIAS) + 1;
        app_context_node->base.name = malloc(name_size);
        if (app_context_node->base.name == NULL)
        {
            archi_application_forget_vtable(app_vtable_node);
            free(app_context_node);

            archi_log_error(M, "Couldn't allocate the application context node name.");
            return ARCHI_ERROR_ALLOC;
        }

        memcpy(app_context_node->base.name, ARCHI_APP_CONTEXT_ALIAS, name_size);
    }

    *app = (archi_application_t){
        .vtables = {.head = (archi_list_node_t*)app_vtable_node,
            .tail = (archi_list_node_t*)app_vtable_node},
        .contexts = {.head = (archi_list_node_t*)app_context_node,
            .tail = (archi_list_node_t*)app_context_node},
    };

    return 0;
}

/*****************************************************************************/

static
archi_status_t
archi_main_plugin_help(
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

    /////////////////////
    // Local variables //
    /////////////////////

    archi_status_t code; // status of operations

    void **shmaddr; // shared memory address
    archi_app_configuration_t *app_config; // application configuration

    // Normal execution mode
    archi_plugin_vtable_t app_vtable; // application virtual table
    struct archi_app_context app_context; // application context

    // Plugin help mode
    union {
        archi_app_config_plugin_list_node_t as_node;
        char as_bytes[sizeof(archi_app_config_plugin_list_node_t) + sizeof(char*) * 1]; // vtable_symbol[] of size 1
    } help_config_plugin_node; // configuration node for the plugin help mode
    archi_app_configuration_t help_config; // plugin help mode configuration

    ////////////////////////////////
    // Print the application logo //
    ////////////////////////////////

    {
        archi_print(ARCHI_COLOR_RESET "\n");
        if (!args->no_logo)
            archi_print(ARCHI_COLOR_FG_GREEN "%s" ARCHI_COLOR_RESET "\n\n", ARCHI_PELAGO_LOGO);
    }

    //////////////////////
    // Preparation step //
    //////////////////////

    if (args->plugin_help.mode) // plugin help mode
    {
        archi_log_debug(M, "Preparing application configuration for the plugin help mode...");

        help_config_plugin_node.as_node = (archi_app_config_plugin_list_node_t){
            .base.name = "plugin", // any valid name will do here
            .pathname = args->plugin_help.pathname,
            .num_vtables = 1,
        };
        help_config_plugin_node.as_node.vtable_symbol[0] = args->plugin_help.vtable_symbol;

        help_config = (archi_app_configuration_t){
            .plugins = {.head = (archi_list_node_t*)&help_config_plugin_node,
                .tail = (archi_list_node_t*)&help_config_plugin_node},
        };

        shmaddr = NULL;
        app_config = &help_config;

        *app = (archi_application_t){0};
    }
    else // normal execution mode
    {
        // Attach to shared memory
        archi_log_debug(M, "Attaching to shared memory...");

        shmaddr = archi_shared_memory_attach(args->config.pathname, args->config.proj_id, false);
        if (shmaddr == NULL)
        {
            archi_log_error(M, "Couldn't attach to shared memory at pathname '%s', project id %i.",
                    args->config.pathname, args->config.proj_id);

            return ARCHI_EXIT_CODE(ARCHI_ERROR_ATTACH);
        }

        app_config = shmaddr[1]; // shmaddr[0] is just shmaddr, smaddr[1] is the pointer to configuration

        // Initialize the application instance with application vtable and context
        {
            app_vtable = (archi_plugin_vtable_t){
                .format = {.magic = ARCHI_API_MAGIC, .version = ARCHI_API_VERSION},
                .info = {.name = ARCHI_APP_CONTEXT_ALIAS, .description = "Global virtual table"},
                .func = {.set_fn = archi_app_vtable_set},
            };

            app_context = (struct archi_app_context){0};
        }

        code = archi_main_prepare_app(app, &app_vtable, &app_context);
        if (code != 0)
            return ARCHI_EXIT_CODE(code);
    }

    /////////////////////////
    // Initialization step //
    /////////////////////////

    {
        // Initialize the application
        archi_log_info(M, "Initializing the application...");

        /***************************************************/
        code = archi_application_initialize(app, app_config);
        /***************************************************/

        // Exit on error
        if (code != 0)
        {
            archi_log_error(M, "Couldn't initialize the application.");
            goto finish;
        }
    }

    ////////////////////
    // Execution step //
    ////////////////////

    if (args->plugin_help.mode) // plugin help mode
    {
        code = archi_main_plugin_help(app, args->plugin_help.topic);
        if (code != 0)
            archi_log_error(M, "Couldn't provide application plugin help.");
    }
    else // normal execution mode
    {
        // Run the application
        if (app_context.entry_state.function == NULL)
        {
            archi_log_error(M, "Application entry state is null.");

            code = ARCHI_ERROR_CONFIG;
            goto finish;
        }

        archi_log_info(M, "Running the application...");

        /***************************************************************************************/
        /***************************************************************************************/
        code = archi_finite_state_machine(app_context.entry_state, app_context.state_transition);
        /***************************************************************************************/
        /***************************************************************************************/
    }

finish:

    ///////////////////////
    // Finalization step //
    ///////////////////////

    {
        // Clean up the resources and return
        archi_log_info(M, "Finalizing the application...");

        /******************************/
        archi_application_finalize(app);
        /******************************/
    }

    // Detach from shared memory
    archi_log_debug(M, "Detaching from shared memory...");

    if (shmaddr != NULL)
        archi_shared_memory_detach(shmaddr);

    return ARCHI_EXIT_CODE(code);
}

