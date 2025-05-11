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

#include "archi/builtin/ds_lfqueue/context.var.h"
#include "archi/ds/lfqueue/api.fun.h"

#include <string.h> // for strcmp()

ARCHI_CONTEXT_INIT_FUNC(archi_context_ds_lfqueue_init)
{
    archi_lfqueue_alloc_params_t lfqueue_alloc_params = {0};

    bool param_struct_set = false;
    bool param_capacity_log2_set = false;
    bool param_element_size_set = false;
    bool param_element_alignment_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("params", params->name) == 0)
        {
            if (param_struct_set)
                continue;
            param_struct_set = true;

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

            lfqueue_alloc_params.capacity_log2 = *(size_t*)params->value.ptr;
        }
        else if (strcmp("element_size", params->name) == 0)
        {
            if (param_element_size_set)
                continue;
            param_element_size_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            lfqueue_alloc_params.element_size = *(size_t*)params->value.ptr;
        }
        else if (strcmp("element_alignment", params->name) == 0)
        {
            if (param_element_alignment_set)
                continue;
            param_element_alignment_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            lfqueue_alloc_params.element_alignment = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_status_t code;
    struct archi_lfqueue *lfqueue = archi_lfqueue_alloc(lfqueue_alloc_params, &code);

    if (code < 0)
        return code;

    context->public_value = (archi_pointer_t){
        .ptr = lfqueue,
        .element = {
            .num_of = 1,
        },
    };
    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_ds_lfqueue_final)
{
    archi_lfqueue_free(context.public_value.ptr);
}

const archi_context_interface_t archi_context_ds_lfqueue_interface = {
    .init_fn = archi_context_ds_lfqueue_init,
    .final_fn = archi_context_ds_lfqueue_final,
};

