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
 * @brief Application context interface for the signal handlers.
 */

#include "archi/ipc_signal/ctx/handler.var.h"
#include "archi/ipc_signal/api/handler.typ.h"
#include "archipelago/base/ref_count.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

struct archi_context_signal_handler_data {
    archi_pointer_t signal_handler;

    archi_pointer_t signal_handler_function;
    archi_pointer_t signal_handler_data;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_signal_handler_init)
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

    struct archi_context_signal_handler_data *context_data = malloc(sizeof(*context_data));
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

    *context_data = (struct archi_context_signal_handler_data){
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

ARCHI_CONTEXT_FINAL_FUNC(archi_context_signal_handler_final)
{
    struct archi_context_signal_handler_data *context_data =
        (struct archi_context_signal_handler_data*)context;

    archi_reference_count_decrement(context_data->signal_handler_function.ref_count);
    archi_reference_count_decrement(context_data->signal_handler_data.ref_count);
    free(context_data->signal_handler.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_signal_handler_get)
{
    struct archi_context_signal_handler_data *context_data =
        (struct archi_context_signal_handler_data*)context;

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

ARCHI_CONTEXT_SET_FUNC(archi_context_signal_handler_set)
{
    struct archi_context_signal_handler_data *context_data =
        (struct archi_context_signal_handler_data*)context;

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

const archi_context_interface_t archi_context_signal_handler_interface = {
    .init_fn = archi_context_signal_handler_init,
    .final_fn = archi_context_signal_handler_final,
    .get_fn = archi_context_signal_handler_get,
    .set_fn = archi_context_signal_handler_set,
};

