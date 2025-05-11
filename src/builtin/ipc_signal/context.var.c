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
#include "archi/util/ref_count.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), strncmp()
#include <stdalign.h> // for alignof()
#include <threads.h> // for mtx_*

#define REF_FUNCTION 0
#define REF_DATA 1
#define NUM_REFERENCES 2

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

    archi_signal_handler_t *signal_handler = malloc(sizeof(*signal_handler));
    if (signal_handler == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *signal_handler = (archi_signal_handler_t){
        .function = (archi_signal_handler_function_t)signal_handler_function.fptr,
        .data = signal_handler_data.ptr,
    };

    context->num_references = NUM_REFERENCES;
    context->reference = malloc(sizeof(*context->reference) * context->num_references);
    if (context->reference == NULL)
    {
        free(signal_handler);
        return ARCHI_STATUS_ENOMEMORY;
    }

    context->reference[REF_FUNCTION] = signal_handler_function;
    context->reference[REF_DATA] = signal_handler_data;

    for (size_t i = 0; i < NUM_REFERENCES; i++)
        archi_reference_count_increment(context->reference[i].ref_count);

    context->public_value = (archi_pointer_t){
        .ptr = signal_handler,
        .element = {
            .num_of = 1,
            .size = sizeof(*signal_handler),
            .alignment = alignof(archi_signal_handler_t),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_ipc_signal_handler_final)
{
    for (size_t i = context.num_references; i-- > 0;)
        archi_reference_count_decrement(context.reference[i].ref_count);

    free(context.reference);
    free(context.public_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_ipc_signal_handler_get)
{
    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_FUNCTION];
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_DATA];
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_ipc_signal_handler_set)
{
    archi_signal_handler_t *signal_handler = context.public_value.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_FUNCTION].ref_count);

        signal_handler->function = (archi_signal_handler_function_t)value.fptr;
        context.reference[REF_FUNCTION] = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_DATA].ref_count);

        signal_handler->data = value.ptr;
        context.reference[REF_DATA] = value;
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

#undef REF_FUNCTION
#undef REF_DATA
#undef NUM_REFERENCES

/*****************************************************************************/

#define ARCHI_CONTEXT_IPC_SIGNAL_HANDLERS_CAPACITY  16 // larger capacity isn't needed, probably

struct archi_context_ipc_signal {
    struct archi_signal_management_context *thread; ///< Signal management thread context.

    struct archi_hashmap *signal_handlers; ///< Hashmap of signal handlers.
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
    struct archi_context_ipc_signal *signal_management = data;

    struct archi_context_ipc_signal_handler_args args = {
        .signo = signo,
        .siginfo = siginfo,
        .flags = flags,

        .set_signal_flag = false,
    };

    {
        mtx_lock(&signal_management->mutex_signal_handlers);

        archi_hashmap_traverse(signal_management->signal_handlers, true,
                archi_context_ipc_signal_management_hashmap_traverse, &args);

        mtx_unlock(&signal_management->mutex_signal_handlers);
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

    struct archi_context_ipc_signal *signal_management = malloc(sizeof(*signal_management));
    if (signal_management == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *signal_management = (struct archi_context_ipc_signal){0};

    if (mtx_init(&signal_management->mutex_signal_handlers, mtx_plain) != thrd_success)
    {
        free(signal_management);
        return ARCHI_STATUS_ERESOURCE;
    }

    signal_management->signal_handlers = archi_hashmap_alloc((archi_hashmap_alloc_params_t){
            .capacity = ARCHI_CONTEXT_IPC_SIGNAL_HANDLERS_CAPACITY}, &code);

    if (signal_management->signal_handlers == NULL)
    {
        mtx_destroy(&signal_management->mutex_signal_handlers);
        free(signal_management);
        return code;
    }

    signal_management->thread = archi_signal_management_start(
            (archi_signal_management_start_params_t){
                .signals = signals,
                .signal_handler = {
                    .function = archi_context_ipc_signal_management_handler,
                    .data = signal_management,
                },
            },
            &code);

    if (signal_management->thread == NULL)
    {
        mtx_destroy(&signal_management->mutex_signal_handlers);
        archi_hashmap_free(signal_management->signal_handlers);
        free(signal_management);
        return code;
    }

    context->public_value = (archi_pointer_t){
        .ptr = signal_management,
        .element = {
            .num_of = 1,
        },
    };

    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_ipc_signal_management_final)
{
    struct archi_context_ipc_signal *signal_management = context.public_value.ptr;

    archi_signal_management_stop(signal_management->thread);
    archi_hashmap_free(signal_management->signal_handlers);
    mtx_destroy(&signal_management->mutex_signal_handlers);
    free(signal_management);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_ipc_signal_management_get)
{
    struct archi_context_ipc_signal *signal_management = context.public_value.ptr;

    if (strcmp("flags", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = archi_signal_management_flags(signal_management->thread),
            .ref_count = context.public_value.ref_count,
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
            mtx_lock(&signal_management->mutex_signal_handlers);

            *value = archi_hashmap_get(signal_management->signal_handlers, &slot.name[8], &code);

            mtx_unlock(&signal_management->mutex_signal_handlers);
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
    struct archi_context_ipc_signal *signal_management = context.public_value.ptr;

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

            mtx_lock(&signal_management->mutex_signal_handlers);

            code = archi_hashmap_set(signal_management->signal_handlers,
                    &slot.name[8], value, params);

            mtx_unlock(&signal_management->mutex_signal_handlers);
        }
        else
        {
            mtx_lock(&signal_management->mutex_signal_handlers);

            code = archi_hashmap_unset(signal_management->signal_handlers,
                    &slot.name[8], (archi_hashmap_unset_params_t){0});

            mtx_unlock(&signal_management->mutex_signal_handlers);
        }

        if (code != 0)
            return code;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

