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
 * @brief Application context interface for hashmaps.
 */

#include "archi/ds_hashmap/ctx/hashmap.var.h"
#include "archi/ds_hashmap/api/hashmap.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()

ARCHI_CONTEXT_INIT_FUNC(archi_context_hashmap_init)
{
    archi_hashmap_alloc_params_t hashmap_alloc_params = {0};
    archi_hashmap_alloc_params_t hashmap_alloc_params_fields = {0};

    bool param_params_set = false;
    bool param_capacity_set = false;

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

            hashmap_alloc_params = *(archi_hashmap_alloc_params_t*)params->value.ptr;
        }
        else if (strcmp("capacity", params->name) == 0)
        {
            if (param_capacity_set)
                continue;
            param_capacity_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            hashmap_alloc_params_fields.capacity = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (param_capacity_set)
        hashmap_alloc_params.capacity = hashmap_alloc_params_fields.capacity;

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;
    archi_hashmap_t hashmap = archi_hashmap_alloc(hashmap_alloc_params, &code);

    if (code < 0)
    {
        free(context_data);
        return code;
    }

    *context_data = (archi_pointer_t){
        .ptr = hashmap,
        .element = {
            .num_of = 1,
        },
    };

    *context = context_data;
    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hashmap_final)
{
    archi_hashmap_free(context->ptr);
    free(context);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hashmap_get)
{
    if (slot.num_indices != 0)
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;
    archi_pointer_t val = archi_hashmap_get(context->ptr, slot.name, &code);

    if (code < 0)
        return code;

    *value = val;
    return code;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hashmap_set)
{
    if (slot.num_indices > 1)
        return ARCHI_STATUS_EMISUSE;
    else if ((slot.num_indices > 0) && (slot.index[0] != 0))
        return ARCHI_STATUS_EMISUSE;

    if (value.ptr != NULL)
    {
        archi_hashmap_set_params_t params = {
            .insertion_allowed = true,
            .update_allowed = (slot.num_indices != 0),
        };

        return archi_hashmap_set(context->ptr, slot.name, value, params);
    }
    else
    {
        archi_hashmap_unset_params_t params = {0};

        return archi_hashmap_unset(context->ptr, slot.name, params);
    }
}

const archi_context_interface_t archi_context_hashmap_interface = {
    .init_fn = archi_context_hashmap_init,
    .final_fn = archi_context_hashmap_final,
    .get_fn = archi_context_hashmap_get,
    .set_fn = archi_context_hashmap_set,
};

