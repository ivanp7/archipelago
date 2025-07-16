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
 * @brief Application context interface for thread group work description.
 */

#include "archi/builtin/res_thread_group/work.var.h"
#include "archi/res/thread_group/api.typ.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

struct archi_context_res_thread_group_work_data {
    archi_pointer_t work;

    // References
    archi_pointer_t work_function;
    archi_pointer_t work_data;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_work_init)
{
    archi_pointer_t work_function = {0};
    archi_pointer_t work_data = {0};
    size_t work_size = 0;

    bool param_function_set = false;
    bool param_data_set = false;
    bool param_size_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("function", params->name) == 0)
        {
            if (param_function_set)
                continue;
            param_function_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EVALUE;

            work_function = params->value;
        }
        else if (strcmp("data", params->name) == 0)
        {
            if (param_data_set)
                continue;
            param_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            work_data = params->value;
        }
        else if (strcmp("size", params->name) == 0)
        {
            if (param_size_set)
                continue;
            param_size_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            work_size = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_res_thread_group_work_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_thread_group_work_t *work = malloc(sizeof(*work));
    if (work == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *work = (archi_thread_group_work_t){
        .function = (archi_thread_group_work_func_t)work_function.fptr,
        .data = work_data.ptr,
        .size = work_size,
    };

    *context_data = (struct archi_context_res_thread_group_work_data){
        .work = {
            .ptr = work,
            .element = {
                .num_of = 1,
                .size = sizeof(*work),
                .alignment = alignof(archi_thread_group_work_t),
            },
        },
        .work_function = work_function,
        .work_data = work_data,
    };

    archi_reference_count_increment(work_function.ref_count);
    archi_reference_count_increment(work_data.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_work_final)
{
    struct archi_context_res_thread_group_work_data *context_data =
        (struct archi_context_res_thread_group_work_data*)context;

    archi_reference_count_decrement(context_data->work_function.ref_count);
    archi_reference_count_decrement(context_data->work_data.ref_count);
    free(context_data->work.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_work_get)
{
    struct archi_context_res_thread_group_work_data *context_data =
        (struct archi_context_res_thread_group_work_data*)context;

    archi_thread_group_work_t *work = context_data->work.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->work_function;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->work_data;
    }
    else if (strcmp("size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &work->size,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(work->size),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_res_thread_group_work_set)
{
    struct archi_context_res_thread_group_work_data *context_data =
        (struct archi_context_res_thread_group_work_data*)context;

    archi_thread_group_work_t *work = context_data->work.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->work_function.ref_count);

        work->function = (archi_thread_group_work_func_t)value.fptr;
        context_data->work_function = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->work_data.ref_count);

        work->data = value.ptr;
        context_data->work_data = value;
    }
    else if (strcmp("size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        work->size = *(size_t*)value.ptr;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_res_thread_group_work_interface = {
    .init_fn = archi_context_res_thread_group_work_init,
    .final_fn = archi_context_res_thread_group_work_final,
    .get_fn = archi_context_res_thread_group_work_get,
    .set_fn = archi_context_res_thread_group_work_set,
};

