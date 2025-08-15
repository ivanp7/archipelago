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
 * @brief Application context interface for lock-free queues.
 */

#include "archi/ds_lfqueue/ctx/lfqueue.var.h"
#include "archi/ds_lfqueue/api/lfqueue.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

struct archi_context_lfqueue_data {
    archi_pointer_t lfqueue;
    archi_lfqueue_alloc_params_t params;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_lfqueue_init)
{
    archi_lfqueue_alloc_params_t lfqueue_alloc_params = {0};
    archi_lfqueue_alloc_params_t lfqueue_alloc_params_fields = {0};

    bool param_params_set = false;
    bool param_capacity_log2_set = false;
    bool param_element_size_set = false;
    bool param_element_alignment_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("params", params->name) == 0)
        {
            if (param_params_set)
                continue;
            param_params_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            lfqueue_alloc_params = *(archi_lfqueue_alloc_params_t*)params->value.ptr;
        }
        else if (strcmp("capacity_log2", params->name) == 0)
        {
            if (param_capacity_log2_set)
                continue;
            param_capacity_log2_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            lfqueue_alloc_params_fields.capacity_log2 = *(size_t*)params->value.ptr;
        }
        else if (strcmp("element_size", params->name) == 0)
        {
            if (param_element_size_set)
                continue;
            param_element_size_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            lfqueue_alloc_params_fields.element_size = *(size_t*)params->value.ptr;
        }
        else if (strcmp("element_alignment", params->name) == 0)
        {
            if (param_element_alignment_set)
                continue;
            param_element_alignment_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            lfqueue_alloc_params_fields.element_alignment = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (param_capacity_log2_set)
        lfqueue_alloc_params.capacity_log2 = lfqueue_alloc_params_fields.capacity_log2;

    if (param_element_size_set)
        lfqueue_alloc_params.element_size = lfqueue_alloc_params_fields.element_size;

    if (param_element_alignment_set)
        lfqueue_alloc_params.element_alignment = lfqueue_alloc_params_fields.element_alignment;

    struct archi_context_lfqueue_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;
    archi_lfqueue_t lfqueue = archi_lfqueue_alloc(lfqueue_alloc_params, &code);

    if (code < 0)
    {
        free(context_data);
        return code;
    }

    *context_data = (struct archi_context_lfqueue_data){
        .lfqueue = {
            .ptr = lfqueue,
            .element = {
                .num_of = 1,
            },
        },
        .params = lfqueue_alloc_params,
    };

    *context = (archi_pointer_t*)context_data;
    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_lfqueue_final)
{
    struct archi_context_lfqueue_data *context_data =
        (struct archi_context_lfqueue_data*)context;

    archi_lfqueue_free(context_data->lfqueue.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_lfqueue_get)
{
    struct archi_context_lfqueue_data *context_data =
        (struct archi_context_lfqueue_data*)context;

    if (strcmp("capacity_log2", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->params.capacity_log2,
            .ref_count = context_data->lfqueue.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->params.capacity_log2),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->params.element_size,
            .ref_count = context_data->lfqueue.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->params.element_size),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_alignment", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->params.element_alignment,
            .ref_count = context_data->lfqueue.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->params.element_alignment),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_lfqueue_interface = {
    .init_fn = archi_context_lfqueue_init,
    .final_fn = archi_context_lfqueue_final,
    .get_fn = archi_context_lfqueue_get,
};

