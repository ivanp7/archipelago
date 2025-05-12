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
 * @brief Application context interface for memory objects.
 */

#include "archi/builtin/mem/context.var.h"
#include "archi/mem/interface.fun.h"

#include <string.h> // for strcmp()
#include <stdbool.h>
#include <stdalign.h>

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_context_memory_init)
{
    archi_pointer_t interface = {0};
    void *alloc_data = NULL;
    archi_array_layout_t layout = {0};

    bool param_interface_set = false;
    bool param_alloc_data_set = false;
    bool param_layout_set = false;
    bool param_num_elements_set = false;
    bool param_element_size_set = false;
    bool param_element_alignment_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("interface", params->name) == 0)
        {
            if (param_interface_set)
                continue;
            param_interface_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            interface = params->value;
        }
        else if (strcmp("alloc_data", params->name) == 0)
        {
            if (param_alloc_data_set)
                continue;
            param_alloc_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            alloc_data = params->value.ptr;
        }
        else if (strcmp("layout", params->name) == 0)
        {
            if (param_layout_set)
                continue;
            param_layout_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            layout = *(archi_array_layout_t*)params->value.ptr;
        }
        else if (strcmp("num_elements", params->name) == 0)
        {
            if (param_num_elements_set)
                continue;
            param_num_elements_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            layout.num_of = *(size_t*)params->value.ptr;
        }
        else if (strcmp("element_size", params->name) == 0)
        {
            if (param_element_size_set)
                continue;
            param_element_size_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            layout.size = *(size_t*)params->value.ptr;
        }
        else if (strcmp("element_alignment", params->name) == 0)
        {
            if (param_element_alignment_set)
                continue;
            param_element_alignment_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            layout.alignment = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_status_t code;

    archi_memory_t memory = archi_memory_allocate(interface, alloc_data, layout, &code);
    if (memory == NULL)
        return code;

    context->public_value = (archi_pointer_t){
        .ptr = memory,
        .element = layout,
    };

    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_memory_final)
{
    archi_memory_free(context.public_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_memory_get)
{
    if (strcmp("interface", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_pointer_t interface = archi_memory_interface(context.public_value.ptr);
        interface.ref_count = context.public_value.ref_count;
        *value = interface;
    }
    else if (strcmp("num_elements", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context.public_value.element.num_of,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context.public_value.element.num_of),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context.public_value.element.size,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context.public_value.element.size),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_alignment", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context.public_value.element.alignment,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context.public_value.element.alignment),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_memory_interface = {
    .init_fn = archi_context_memory_init,
    .final_fn = archi_context_memory_final,
    .get_fn = archi_context_memory_get,
};

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_context_memory_mapping_init)
{
    archi_pointer_t memory = {0};
    void *map_data = NULL;
    size_t offset = 0;
    size_t num_of = 0;
    bool writeable = false;

    bool param_memory_set = false;
    bool param_map_data_set = false;
    bool param_offset_set = false;
    bool param_num_elements_set = false;
    bool param_writeable_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("memory", params->name) == 0)
        {
            if (param_memory_set)
                continue;
            param_memory_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            memory = params->value;
        }
        else if (strcmp("map_data", params->name) == 0)
        {
            if (param_map_data_set)
                continue;
            param_map_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            map_data = params->value.ptr;
        }
        else if (strcmp("offset", params->name) == 0)
        {
            if (param_offset_set)
                continue;
            param_offset_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            offset = *(size_t*)params->value.ptr;
        }
        else if (strcmp("num_elements", params->name) == 0)
        {
            if (param_num_elements_set)
                continue;
            param_num_elements_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            num_of = *(size_t*)params->value.ptr;
        }
        else if (strcmp("writeable", params->name) == 0)
        {
            if (param_writeable_set)
                continue;
            param_writeable_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            writeable = *(bool*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_status_t code;

    archi_pointer_t mapping = archi_memory_map(memory.ptr, map_data, offset, num_of, writeable, &code);
    if (mapping.ptr == NULL)
        return code;

    archi_reference_count_increment(memory.ref_count);

    context->public_value = mapping;
    context->private_value = memory;

    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_memory_mapping_final)
{
    archi_memory_unmap(context.private_value.ptr);
    archi_reference_count_decrement(context.private_value.ref_count);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_memory_mapping_get)
{
    if (strcmp("memory", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_pointer_t memory = context.private_value;
        memory.ref_count = context.public_value.ref_count;
        *value = memory;
    }
    else if (strcmp("num_elements", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context.public_value.element.num_of,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context.public_value.element.num_of),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context.public_value.element.size,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context.public_value.element.size),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_alignment", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context.public_value.element.alignment,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context.public_value.element.alignment),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_memory_mapping_interface = {
    .init_fn = archi_context_memory_mapping_init,
    .final_fn = archi_context_memory_mapping_final,
    .get_fn = archi_context_memory_mapping_get,
};

