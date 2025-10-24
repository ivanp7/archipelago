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
 * @brief Application context interface for data of memory copying HSP state.
 */

#include "archi/memory/ctx_hsp/copy_data.var.h"
#include "archi/memory/hsp/copy.typ.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

struct archi_context_memory_map_copy_unmap_data_data {
    archi_pointer_t copy_data;

    // References
    archi_pointer_t memory_dest;
    archi_pointer_t map_data_dest;
    archi_pointer_t memory_src;
    archi_pointer_t map_data_src;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_memory_map_copy_unmap_data_init)
{
    archi_pointer_t copy_memory_dest = {0};
    archi_pointer_t copy_map_data_dest = {0};
    archi_pointer_t copy_memory_src = {0};
    archi_pointer_t copy_map_data_src = {0};
    size_t copy_offset_dest = 0;
    size_t copy_offset_src = 0;
    size_t copy_num_of = 0;

    bool param_memory_dest_set = false;
    bool param_map_data_dest_set = false;
    bool param_offset_dest_set = false;
    bool param_memory_src_set = false;
    bool param_map_data_src_set = false;
    bool param_offset_src_set = false;
    bool param_num_of_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("memory_dest", params->name) == 0)
        {
            if (param_memory_dest_set)
                continue;
            param_memory_dest_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            copy_memory_dest = params->value;
        }
        else if (strcmp("map_data_dest", params->name) == 0)
        {
            if (param_map_data_dest_set)
                continue;
            param_map_data_dest_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            copy_map_data_dest = params->value;
        }
        else if (strcmp("offset_dest", params->name) == 0)
        {
            if (param_offset_dest_set)
                continue;
            param_offset_dest_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            copy_offset_dest = *(size_t*)params->value;
        }
        else if (strcmp("memory_src", params->name) == 0)
        {
            if (param_memory_src_set)
                continue;
            param_memory_src_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            copy_memory_src = params->value;
        }
        else if (strcmp("map_data_src", params->name) == 0)
        {
            if (param_map_data_src_set)
                continue;
            param_map_data_src_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            copy_map_data_src = params->value;
        }
        else if (strcmp("offset_src", params->name) == 0)
        {
            if (param_offset_src_set)
                continue;
            param_offset_src_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            copy_offset_src = *(size_t*)params->value;
        }
        else if (strcmp("num_of", params->name) == 0)
        {
            if (param_num_of_set)
                continue;
            param_num_of_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            copy_num_of = *(size_t*)params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_memory_map_copy_unmap_data_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_memory_map_copy_unmap_data_t *copy_data = malloc(sizeof(*copy_data));
    if (copy_data == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *copy_data = (archi_memory_map_copy_unmap_data_t){
        .memory_dest = copy_memory_dest.ptr,
        .offset_dest = copy_offset_dest,
        .map_data_dest = copy_map_data_dest.ptr,
        .memory_src = copy_memory_src.ptr,
        .offset_src = copy_offset_src,
        .map_data_src = copy_map_data_src.ptr,
        .num_of = copy_num_of,
    };

    *context_data = (struct archi_context_memory_map_copy_unmap_data_data){
        .copy_data = {
            .ptr = copy_data,
            .element = {
                .num_of = 1,
                .size = sizeof(*copy_data),
                .alignment = alignof(archi_memory_map_copy_unmap_data_t),
            },
        },
        .memory_dest = copy_memory_dest,
        .map_data_dest = copy_map_data_dest,
        .memory_src = copy_memory_src,
        .map_data_src = copy_map_data_src,
    };

    archi_reference_count_increment(copy_memory_dest.ref_count);
    archi_reference_count_increment(copy_map_data_dest.ref_count);
    archi_reference_count_increment(copy_memory_src.ref_count);
    archi_reference_count_increment(copy_map_data_src.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_memory_map_copy_unmap_data_final)
{
    struct archi_context_memory_map_copy_unmap_data_data *context_data =
        (struct archi_context_memory_map_copy_unmap_data_data*)context;

    archi_reference_count_decrement(context_data->memory_dest.ref_count);
    archi_reference_count_decrement(context_data->map_data_dest.ref_count);
    archi_reference_count_decrement(context_data->memory_src.ref_count);
    archi_reference_count_decrement(context_data->map_data_src.ref_count);

    free(context_data->copy_data.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_memory_map_copy_unmap_data_get)
{
    struct archi_context_memory_map_copy_unmap_data_data *context_data =
        (struct archi_context_memory_map_copy_unmap_data_data*)context;

    archi_memory_map_copy_unmap_data_t *copy_data = context_data->copy_data.ptr;

    if (strcmp("memory_dest", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->memory_dest;
    }
    else if (strcmp("offset_dest", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &copy_data->offset_dest,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(copy_data->offset_dest),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("map_data_dest", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->map_data_dest;
    }
    else if (strcmp("memory_src", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->memory_src;
    }
    else if (strcmp("offset_src", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &copy_data->offset_src,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(copy_data->offset_src),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("map_data_src", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->map_data_src;
    }
    else if (strcmp("num_of", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &copy_data->num_of,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(copy_data->num_of),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_memory_map_copy_unmap_data_set)
{
    struct archi_context_memory_map_copy_unmap_data_data *context_data =
        (struct archi_context_memory_map_copy_unmap_data_data*)context;

    archi_memory_map_copy_unmap_data_t *copy_data = context_data->copy_data.ptr;

    if (strcmp("memory_dest", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->memory_dest.ref_count);

        copy_data->memory_dest = value.ptr;
        context_data->memory_dest = value;
    }
    else if (strcmp("offset_dest", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        copy_data->offset_dest = *(size_t*)value.ptr;
    }
    else if (strcmp("map_data_dest", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->map_data_dest.ref_count);

        copy_data->map_data_dest = value.ptr;
        context_data->map_data_dest = value;
    }
    else if (strcmp("memory_src", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->memory_src.ref_count);

        copy_data->memory_src = value.ptr;
        context_data->memory_src = value;
    }
    else if (strcmp("offset_src", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        copy_data->offset_src = *(size_t*)value.ptr;
    }
    else if (strcmp("map_data_src", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->map_data_src.ref_count);

        copy_data->map_data_src = value.ptr;
        context_data->map_data_src = value;
    }
    else if (strcmp("num_of", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        copy_data->num_of = *(size_t*)value.ptr;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_memory_map_copy_unmap_data_interface = {
    .init_fn = archi_context_memory_map_copy_unmap_data_init,
    .final_fn = archi_context_memory_map_copy_unmap_data_final,
    .get_fn = archi_context_memory_map_copy_unmap_data_get,
    .set_fn = archi_context_memory_map_copy_unmap_data_set,
};

