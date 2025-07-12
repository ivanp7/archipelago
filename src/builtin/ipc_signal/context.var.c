/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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
 * @brief Application context interface for the signal management.
 */

#include "archi/builtin/ipc_signal/context.var.h"
#include "archi/ipc/signal/api.fun.h"
#include "archi/ds/hashmap/api.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), strncmp()
#include <stdalign.h> // for alignof()
#include <threads.h> // for mtx_*

struct archi_context_ipc_signal_handler_data {
    archi_pointer_t signal_handler;

    archi_pointer_t signal_handler_function;
    archi_pointer_t signal_handler_data;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_ipc_signal_handler_init)
{
    archi_pointer_t signal_handler_function = {0};
    archi_pointer_t signal_handler_data = {0};

    bool param_function_set = false;
    bool param_data_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("function", params->name) == 0)
        {
            if (param_function_set)
                continue;
            param_function_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EVALUE;

            signal_handler_function = params->value;
        }
        else if (strcmp("data", params->name) == 0)
        {
            if (param_data_set)
                continue;
            param_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            signal_handler_data = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_ipc_signal_handler_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_signal_handler_t *signal_handler = malloc(sizeof(*signal_handler));
    if (signal_handler == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *signal_handler = (archi_signal_handler_t){
        .function = (archi_signal_handler_function_t)signal_handler_function.fptr,
        .data = signal_handler_data.ptr,
    };

    *context_data = (struct archi_context_ipc_signal_handler_data){
        .signal_handler = {
            .ptr = signal_handler,
            .element = {
                .num_of = 1,
                .size = sizeof(*signal_handler),
                .alignment = alignof(archi_signal_handler_t),
            },
        },
        .signal_handler_function = signal_handler_function,
        .signal_handler_data = signal_handler_data,
    };

    archi_reference_count_increment(signal_handler_function.ref_count);
    archi_reference_count_increment(signal_handler_data.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_ipc_signal_handler_final)
{
    struct archi_context_ipc_signal_handler_data *context_data =
        (struct archi_context_ipc_signal_handler_data*)context;

    archi_reference_count_decrement(context_data->signal_handler_function.ref_count);
    archi_reference_count_decrement(context_data->signal_handler_data.ref_count);
    free(context_data->signal_handler.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_ipc_signal_handler_get)
{
    struct archi_context_ipc_signal_handler_data *context_data =
        (struct archi_context_ipc_signal_handler_data*)context;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->signal_handler_function;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->signal_handler_data;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_ipc_signal_handler_set)
{
    struct archi_context_ipc_signal_handler_data *context_data =
        (struct archi_context_ipc_signal_handler_data*)context;

    archi_signal_handler_t *signal_handler = context_data->signal_handler.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->signal_handler_function.ref_count);

        signal_handler->function = (archi_signal_handler_function_t)value.fptr;
        context_data->signal_handler_function = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->signal_handler_data.ref_count);

        signal_handler->data = value.ptr;
        context_data->signal_handler_data = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_ipc_signal_handler_interface = {
    .init_fn = archi_context_ipc_signal_handler_init,
    .final_fn = archi_context_ipc_signal_handler_final,
    .get_fn = archi_context_ipc_signal_handler_get,
    .set_fn = archi_context_ipc_signal_handler_set,
};

/*****************************************************************************/

#define ARCHI_CONTEXT_IPC_SIGNAL_HANDLERS_CAPACITY  32 // larger capacity isn't needed, probably

struct archi_context_ipc_signal_management_data {
    archi_pointer_t context; ///< Signal management context.

    archi_hashmap_t signal_handlers; ///< Hashmap of signal handlers.
    mtx_t mutex_signal_handlers; ///< Mutex for the hashmap.
};

struct archi_context_ipc_signal_handler_args {
    int signo;
    void *siginfo;
    archi_signal_flags_t *flags;

    bool set_signal_flag; // return value
};

static
ARCHI_HASHMAP_TRAV_KV_FUNC(archi_context_ipc_signal_management_hashmap_traverse)
{
    (void) key;
    (void) index;

    struct archi_context_ipc_signal_handler_args *args = data;
    archi_signal_handler_t *signal_handler = value.ptr;

    if (signal_handler->function != NULL)
    {
        bool set_signal_flag = signal_handler->function(
                args->signo, args->siginfo, args->flags, signal_handler->data);

        args->set_signal_flag = args->set_signal_flag || set_signal_flag;
    }

    return (archi_hashmap_trav_action_t){0};
}

static
ARCHI_SIGNAL_HANDLER_FUNC(archi_context_ipc_signal_management_handler)
{
    struct archi_context_ipc_signal_management_data *context_data = data;

    struct archi_context_ipc_signal_handler_args args = {
        .signo = signo,
        .siginfo = siginfo,
        .flags = flags,

        .set_signal_flag = false,
    };

    {
        mtx_lock(&context_data->mutex_signal_handlers);

        archi_hashmap_traverse(context_data->signal_handlers, true,
                archi_context_ipc_signal_management_hashmap_traverse, &args);

        mtx_unlock(&context_data->mutex_signal_handlers);
    }

    return args.set_signal_flag;
}

ARCHI_CONTEXT_INIT_FUNC(archi_context_ipc_signal_management_init)
{
    archi_signal_watch_set_t *signals = NULL;

    bool param_signals_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("signals", params->name) == 0)
        {
            if (param_signals_set)
                continue;
            param_signals_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            signals = params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_status_t code;

    struct archi_context_ipc_signal_management_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    mtx_t mutex_signal_handlers;
    if (mtx_init(&mutex_signal_handlers, mtx_plain) != thrd_success)
    {
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    archi_hashmap_t signal_handlers = archi_hashmap_alloc((archi_hashmap_alloc_params_t){
            .capacity = ARCHI_CONTEXT_IPC_SIGNAL_HANDLERS_CAPACITY}, &code);
    if (signal_handlers == NULL)
    {
        mtx_destroy(&mutex_signal_handlers);
        free(context_data);
        return code;
    }

    archi_signal_management_context_t signal_management = archi_signal_management_start(
            (archi_signal_management_start_params_t){
                .signals = signals,
                .signal_handler = {
                    .function = archi_context_ipc_signal_management_handler,
                    .data = context_data,
                },
            },
            &code);
    if (signal_management == NULL)
    {
        archi_hashmap_free(context_data->signal_handlers);
        mtx_destroy(&mutex_signal_handlers);
        free(context_data);
        return code;
    }

    *context_data = (struct archi_context_ipc_signal_management_data){
        .context = {
            .ptr = signal_management,
            .element = {
                .num_of = 1,
            },
        },
        .signal_handlers = signal_handlers,
        .mutex_signal_handlers = mutex_signal_handlers,
    };

    *context = (archi_pointer_t*)context_data;
    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_ipc_signal_management_final)
{
    struct archi_context_ipc_signal_management_data *context_data =
        (struct archi_context_ipc_signal_management_data*)context;

    archi_signal_management_stop(context_data->context.ptr);
    archi_hashmap_free(context_data->signal_handlers);
    mtx_destroy(&context_data->mutex_signal_handlers);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_ipc_signal_management_get)
{
    struct archi_context_ipc_signal_management_data *context_data =
        (struct archi_context_ipc_signal_management_data*)context;

    if (strcmp("flags", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = archi_signal_management_flags(context_data->context.ptr),
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
                .size = ARCHI_SIGNAL_FLAGS_SIZEOF,
                .alignment = alignof(archi_signal_flags_t),
            },
        };
    }
    else if (strncmp("handler.", slot.name, 8) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_status_t code;

        {
            mtx_lock(&context_data->mutex_signal_handlers);

            *value = archi_hashmap_get(context_data->signal_handlers, &slot.name[8], &code);

            mtx_unlock(&context_data->mutex_signal_handlers);
        }

        if (code != 0)
            return code;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_ipc_signal_management_set)
{
    struct archi_context_ipc_signal_management_data *context_data =
        (struct archi_context_ipc_signal_management_data*)context;

    if (strncmp("handler.", slot.name, 8) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_status_t code;

        if (value.ptr != NULL)
        {
            archi_hashmap_set_params_t params = {
                .insertion_allowed = true,
                .update_allowed = true,
            };

            {
                mtx_lock(&context_data->mutex_signal_handlers);

                code = archi_hashmap_set(context_data->signal_handlers, &slot.name[8], value, params);

                mtx_unlock(&context_data->mutex_signal_handlers);
            }
        }
        else
        {
            mtx_lock(&context_data->mutex_signal_handlers);

            code = archi_hashmap_unset(context_data->signal_handlers,
                    &slot.name[8], (archi_hashmap_unset_params_t){0});

            mtx_unlock(&context_data->mutex_signal_handlers);
        }

        if (code != 0)
            return code;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

