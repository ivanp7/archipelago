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
 * @brief Application context interface.
 */

#include "archi/exe/context.fun.h"
#include "archi/exe/context.typ.h"
#include "archi/app/config.fun.h"
#include "archi/app/context.fun.h"
#include "archi/app/loader.fun.h"
#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"
#include "archi/util/flexible.def.h"
#include "archi/util/list.fun.h"
#include "archi/util/os/signal.fun.h"

#include "archi/plugin/files/context.fun.h"
#include "archi/plugin/shared_memory/context.fun.h"
#include "archi/plugin/shared_libraries/context.fun.h"
#include "archi/plugin/threads/context.fun.h"

#include <string.h>

#define CHECK_STRUCT(struct_type) do { \
    if ((value->type != ARCHI_VALUE_DATA) || (value->ptr == NULL) || (value->num_of == 0) || \
            ((value->size != 0) && (value->size < sizeof(struct_type)))) \
    return ARCHI_ERROR_MISUSE; \
} while (0)

#define CHECK_FUNCTION() do { \
    if (value->type != ARCHI_VALUE_FUNCTION) \
        return ARCHI_ERROR_MISUSE; \
} while (0)

#define CHECK_DATA() do { \
    if (value->type != ARCHI_VALUE_DATA) \
        return ARCHI_ERROR_MISUSE; \
} while (0)

#define REGISTER_PLUGIN(alias, interface) do { \
    code = archi_container_insert(app->plugin_context_interfaces.container, \
            (alias), (void*)(interface)); \
    if (code != 0) \
        return code; \
} while (0)

ARCHI_CONTEXT_SET_FUNC(archi_app_context_set)
{
    if ((context == NULL) || (slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_app_context_t *app_context = context;

    if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE) == 0)
    {
        CHECK_STRUCT(archi_state_t);
        memcpy(&app_context->entry_state, value->ptr, sizeof(app_context->entry_state));
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE_FUNC) == 0)
    {
        CHECK_FUNCTION();
        app_context->entry_state.function = (archi_state_function_t)value->fptr;
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE_DATA) == 0)
    {
        CHECK_DATA();
        app_context->entry_state.data = value->ptr;
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE_METADATA) == 0)
    {
        CHECK_DATA();
        app_context->entry_state.metadata = value->ptr;
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_TRANSITION) == 0)
    {
        CHECK_STRUCT(archi_transition_t);
        memcpy(&app_context->transition, value->ptr, sizeof(app_context->transition));
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_TRANSITION_FUNC) == 0)
    {
        CHECK_FUNCTION();
        app_context->transition.function = (archi_transition_function_t)value->fptr;
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_TRANSITION_DATA) == 0)
    {
        CHECK_DATA();
        app_context->transition.data = value->ptr;
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_SIGNAL_HANDLER) == 0)
    {
        CHECK_STRUCT(archi_signal_handler_t);
        memcpy(&app_context->signal_handler, value->ptr, sizeof(app_context->signal_handler));
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_SIGNAL_HANDLER_FUNC) == 0)
    {
        CHECK_FUNCTION();
        app_context->signal_handler.function = (archi_signal_handler_function_t)value->fptr;
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_SIGNAL_HANDLER_DATA) == 0)
    {
        CHECK_DATA();
        app_context->signal_handler.data = value->ptr;
    }
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

ARCHI_CONTEXT_GET_FUNC(archi_app_context_get)
{
    if ((context == NULL) || (slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_app_context_t *app_context = context;

    if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE) == 0)
    {
        *value = (archi_value_t){
            .ptr = &app_context->entry_state,
            .size = sizeof(app_context->entry_state),
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE_FUNC) == 0)
    {
        *value = (archi_value_t){
            .fptr = (archi_function_t)app_context->entry_state.function,
            .num_of = 1,
            .type = ARCHI_VALUE_FUNCTION,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE_DATA) == 0)
    {
        *value = (archi_value_t){
            .ptr = &app_context->entry_state.data,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE_METADATA) == 0)
    {
        *value = (archi_value_t){
            .ptr = &app_context->entry_state.metadata,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_TRANSITION) == 0)
    {
        *value = (archi_value_t){
            .ptr = &app_context->transition,
            .size = sizeof(app_context->transition),
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_TRANSITION_FUNC) == 0)
    {
        *value = (archi_value_t){
            .fptr = (archi_function_t)app_context->transition.function,
            .num_of = 1,
            .type = ARCHI_VALUE_FUNCTION,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_TRANSITION_DATA) == 0)
    {
        *value = (archi_value_t){
            .ptr = &app_context->transition.data,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_SIGNAL_HANDLER) == 0)
    {
        *value = (archi_value_t){
            .ptr = &app_context->signal_handler,
            .size = sizeof(app_context->signal_handler),
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_SIGNAL_HANDLER_FUNC) == 0)
    {
        *value = (archi_value_t){
            .fptr = (archi_function_t)app_context->signal_handler.function,
            .num_of = 1,
            .type = ARCHI_VALUE_FUNCTION,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_SIGNAL_HANDLER_DATA) == 0)
    {
        *value = (archi_value_t){
            .ptr = &app_context->signal_handler.data,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_CONTEXT_SLOT_SIGNAL_FLAGS) == 0)
    {
        *value = (archi_value_t){
            .ptr = app_context->signal_flags,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

const archi_context_interface_t archi_app_context_interface = {
    .set_fn = archi_app_context_set,
    .get_fn = archi_app_context_get,
};

/*****************************************************************************/

void
archi_app_parse_shm(
        void **restrict shmaddr,

        const archi_signal_watch_set_t *restrict *restrict signal_watch_set,
        archi_container_t *restrict plugin_libraries,
        archi_container_t *restrict plugin_interfaces,
        archi_container_t *restrict app_config_steps)
{
    if ((shmaddr == NULL) || (shmaddr[ARCHI_SHM_ADDR] != shmaddr))
        return;

    if (signal_watch_set != NULL)
        *signal_watch_set = shmaddr[ARCHI_SHM_PTR_SIGNAL_WATCH_SET];

    if (plugin_libraries != NULL)
        *plugin_libraries = (archi_container_t){
            .data = shmaddr[ARCHI_SHM_PTR_PLUGIN_LIBRARIES],
            .interface = &archi_list_container_interface};

    if (plugin_interfaces != NULL)
        *plugin_interfaces = (archi_container_t){
            .data = shmaddr[ARCHI_SHM_PTR_PLUGIN_INTERFACES],
            .interface = &archi_list_container_interface};

    if (app_config_steps != NULL)
        *app_config_steps = (archi_container_t){
            .data = shmaddr[ARCHI_SHM_PTR_APP_CONFIG_STEPS],
            .interface = &archi_list_container_interface};
}

static
ARCHI_SIGNAL_HANDLER_FUNC(archi_app_signal_handler_func)
{
    if (data == NULL)
        return true;

    archi_app_signal_handler_spinlock_t *signal_handler_spinlock = data;

    // Get current signal handler
    archi_signal_handler_t signal_handler = {0};
    {
        while (atomic_flag_test_and_set_explicit(&signal_handler_spinlock->spinlock,
                    memory_order_acquire)); // lock

        signal_handler = signal_handler_spinlock->signal_handler;

        atomic_flag_clear_explicit(&signal_handler_spinlock->spinlock,
                memory_order_release); // unlock
    }

    // Call the signal handler function
    if (signal_handler.function != NULL)
        return signal_handler.function(signo, siginfo, signals, signal_handler.data);
    else
        return true;
}

archi_status_t
archi_app_initialize(
        archi_application_t *app,

        const archi_signal_watch_set_t *signal_watch_set)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    // Initialize object fields
    *app = (archi_application_t){
        .context_handle_interface = {
            .handle = &app->context,
            .interface = &archi_app_context_interface,
        },
        .context_node = {
            .base.name = "",
            .value = {.ptr = &app->context_handle_interface, .num_of = 1, .type = ARCHI_VALUE_DATA},
        },

        .plugin_contexts = {
            .container_data.list = {
                .head = (archi_list_node_t*)&app->context_node,
                .tail = (archi_list_node_t*)&app->context_node,
            },
            .container = {
                .data = &app->plugin_contexts.container_data,
                .interface = &archi_list_container_interface,
            },
        },
        .plugin_context_interfaces = {
            .container = {
                .data = &app->plugin_context_interfaces.container_data,
                .interface = &archi_list_container_interface,
            },
        },
        .plugin_libraries = {
            .container = {
                .data = &app->plugin_libraries.container_data,
                .interface = &archi_list_container_interface,
            },
        },

        .signal_handler_spinlock = {
            .spinlock = ATOMIC_FLAG_INIT,
        },
    };

    // Fill list of context interfaces with built-in plugins
    REGISTER_PLUGIN(ARCHI_FILE_CONTEXT_INTERFACE_ALIAS,
            &archi_file_context_interface);
    REGISTER_PLUGIN(ARCHI_SHARED_LIBRARY_CONTEXT_INTERFACE_ALIAS,
            &archi_shared_library_context_interface);
    REGISTER_PLUGIN(ARCHI_SHARED_MEMORY_CONTEXT_INTERFACE_ALIAS,
            &archi_shared_memory_context_interface);
    REGISTER_PLUGIN(ARCHI_SHARED_LIBRARY_CONTEXT_INTERFACE_ALIAS,
            &archi_shared_library_context_interface);

    // Start signal management
    if (signal_watch_set != NULL)
    {
        app->signal_management = archi_signal_management_thread_start(signal_watch_set,
                (archi_signal_handler_t){.function = archi_app_signal_handler_func,
                .data = &app->signal_handler_spinlock});

        if (app->signal_management == NULL)
            return ARCHI_ERROR_SIGNAL;

        archi_signal_management_thread_get_properties(app->signal_management,
                &app->context.signal_flags, NULL);
    }

    return 0;
}

archi_status_t
archi_app_configure(
        archi_application_t *app,

        archi_container_t plugin_libraries,
        archi_container_t plugin_interfaces,
        archi_container_t app_config_steps)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    // Load plugin libraries
    code = archi_container_traverse(plugin_libraries,
            archi_app_load_library, &app->plugin_libraries.container);
    if (code != 0)
        goto failure;

    // Get plugin context interfaces
    {
        archi_app_get_library_content_data_t get_library_content_data = {
            .contents = app->plugin_context_interfaces.container,
            .loaded_libraries = app->plugin_libraries.container,
        };

        code = archi_container_traverse(plugin_interfaces,
                archi_app_get_library_content, &get_library_content_data);
        if (code != 0)
            goto failure;
    }

    // Configure the application
    {
        archi_app_apply_config_step_data_t apply_config_step_data = {
            .contexts = app->plugin_contexts.container,
            .context_interfaces = app->plugin_context_interfaces.container,
        };

        code = archi_container_traverse(app_config_steps,
                archi_app_apply_config_step, &apply_config_step_data);
        if (code != 0)
            goto failure;
    }

    // Set signal handler
    {
        while (atomic_flag_test_and_set_explicit(&app->signal_handler_spinlock.spinlock,
                    memory_order_acquire)); // lock

        app->signal_handler_spinlock.signal_handler = app->context.signal_handler;

        atomic_flag_clear_explicit(&app->signal_handler_spinlock.spinlock,
                memory_order_release); // unlock
    }

    return 0;

failure:
    archi_app_finalize(app);
    return code;
}

static
ARCHI_CONTAINER_ELEMENT_FUNC(archi_app_finalize_context)
{
    (void) key;
    (void) data;

    archi_context_t *context = element;
    archi_context_finalize(context->handle);

    return 0;
}

void
archi_app_finalize(
        archi_application_t *app)
{
    if (app == NULL)
        return;

    // Finalize contexts
    archi_container_traverse(app->plugin_contexts.container,
            archi_app_finalize_context, NULL);

    // Deallocate list of contexts
    archi_list_remove_nodes(&app->plugin_contexts.container_data.list, NULL, NULL,
            archi_list_act_func_free_named, NULL,
            app->plugin_contexts.container_data.traverse_from_head, 0, NULL);

    // Deallocate list of context interfaces
    archi_list_remove_nodes(&app->plugin_context_interfaces.container_data.list, NULL, NULL,
            archi_list_act_func_free_named, NULL,
            app->plugin_context_interfaces.container_data.traverse_from_head, 0, NULL);

    // Unload plugin libraries
    archi_container_traverse(app->plugin_libraries.container,
            archi_app_unload_library, NULL);

    // Deallocate list of context interfaces
    archi_list_remove_nodes(&app->plugin_libraries.container_data.list, NULL, NULL,
            archi_list_act_func_free_named, NULL,
            app->plugin_libraries.container_data.traverse_from_head, 0, NULL);

    // Stop signal management
    archi_signal_management_thread_stop(app->signal_management);

    // Unset object fields
    *app = (archi_application_t){0};
}

