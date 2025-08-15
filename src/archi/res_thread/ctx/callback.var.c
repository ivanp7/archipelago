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
 * @brief Application context interface for thread group callbacks.
 */

#include "archi/res_thread/ctx/callback.var.h"
#include "archi/res_thread/api/callback.typ.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

struct archi_context_thread_group_callback_data {
    archi_pointer_t callback;

    // References
    archi_pointer_t callback_function;
    archi_pointer_t callback_data;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_thread_group_callback_init)
{
    archi_pointer_t callback_function = {0};
    archi_pointer_t callback_data = {0};

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

            callback_function = params->value;
        }
        else if (strcmp("data", params->name) == 0)
        {
            if (param_data_set)
                continue;
            param_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            callback_data = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_thread_group_callback_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_thread_group_callback_t *callback = malloc(sizeof(*callback));
    if (callback == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *callback = (archi_thread_group_callback_t){
        .function = (archi_thread_group_callback_func_t)callback_function.fptr,
        .data = callback_data.ptr,
    };

    *context_data = (struct archi_context_thread_group_callback_data){
        .callback = {
            .ptr = callback,
            .element = {
                .num_of = 1,
                .size = sizeof(*callback),
                .alignment = alignof(archi_thread_group_callback_t),
            },
        },
        .callback_function = callback_function,
        .callback_data = callback_data,
    };

    archi_reference_count_increment(callback_function.ref_count);
    archi_reference_count_increment(callback_data.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_thread_group_callback_final)
{
    struct archi_context_thread_group_callback_data *context_data =
        (struct archi_context_thread_group_callback_data*)context;

    archi_reference_count_decrement(context_data->callback_function.ref_count);
    archi_reference_count_decrement(context_data->callback_data.ref_count);
    free(context_data->callback.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_thread_group_callback_get)
{
    struct archi_context_thread_group_callback_data *context_data =
        (struct archi_context_thread_group_callback_data*)context;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->callback_function;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->callback_data;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_thread_group_callback_set)
{
    struct archi_context_thread_group_callback_data *context_data =
        (struct archi_context_thread_group_callback_data*)context;

    archi_thread_group_callback_t *callback = context_data->callback.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->callback_function.ref_count);

        callback->function = (archi_thread_group_callback_func_t)value.fptr;
        context_data->callback_function = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->callback_data.ref_count);

        callback->data = value.ptr;
        context_data->callback_data = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_thread_group_callback_interface = {
    .init_fn = archi_context_thread_group_callback_init,
    .final_fn = archi_context_thread_group_callback_final,
    .get_fn = archi_context_thread_group_callback_get,
    .set_fn = archi_context_thread_group_callback_set,
};

