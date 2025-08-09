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
 * @brief Application context interface for memory.
 */

#include "archi/builtin/mem/memory.var.h"
#include "archi/mem/interface.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdbool.h>
#include <stdalign.h> // for alignof()

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_context_memory_init)
{
    archi_pointer_t interface = {0};
    void *alloc_data = NULL;
    archi_array_layout_t layout = {0};
    archi_array_layout_t layout_fields = {0};

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

            layout_fields.num_of = *(size_t*)params->value.ptr;
        }
        else if (strcmp("element_size", params->name) == 0)
        {
            if (param_element_size_set)
                continue;
            param_element_size_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            layout_fields.size = *(size_t*)params->value.ptr;
        }
        else if (strcmp("element_alignment", params->name) == 0)
        {
            if (param_element_alignment_set)
                continue;
            param_element_alignment_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            layout_fields.alignment = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (param_num_elements_set)
        layout.num_of = layout_fields.num_of;

    if (param_element_size_set)
        layout.size = layout_fields.size;

    if (param_element_alignment_set)
        layout.alignment = layout_fields.alignment;

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;

    archi_memory_t memory = archi_memory_allocate(interface, alloc_data, layout, &code);
    if (memory == NULL)
    {
        free(context_data);
        return code;
    }

    *context_data = (archi_pointer_t){
        .ptr = memory,
        .element = layout,
    };

    *context = context_data;
    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_memory_final)
{
    archi_memory_free(context->ptr);
    free(context);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_memory_get)
{
    if (strcmp("interface", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = archi_memory_interface(context->ptr);
    }
    else if (strcmp("allocation", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = archi_memory_allocation(context->ptr);
    }
    else if (strcmp("layout", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context->element,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context->element),
                .alignment = alignof(archi_array_layout_t),
            },
        };
    }
    else if (strcmp("num_elements", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context->element.num_of,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context->element.num_of),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context->element.size,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context->element.size),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_alignment", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context->element.alignment,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context->element.alignment),
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

