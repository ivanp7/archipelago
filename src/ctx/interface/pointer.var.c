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
 * @brief Context interface for pointer wrappers.
 */

#include "archi/ctx/interface/pointer.var.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

ARCHI_CONTEXT_INIT_FUNC(archi_context_pointer_init)
{
    archi_pointer_t value = {0};
    uint64_t flags = 0;
    archi_array_layout_t layout = {0};
    archi_array_layout_t layout_fields = {0};

    bool param_value_set = false;
    bool param_flags_set = false;
    bool param_layout_set = false;
    bool param_num_elements_set = false;
    bool param_element_size_set = false;
    bool param_element_alignment_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("value", params->name) == 0)
        {
            if (param_value_set)
                continue;
            param_value_set = true;

            value = params->value;
        }
        else if (strcmp("flags", params->name) == 0)
        {
            if (param_flags_set)
                continue;
            param_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            flags = *(uint64_t*)params->value.ptr;
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

    if (param_flags_set)
        value.flags = flags;

    if (param_layout_set)
        value.element = layout;

    if (param_num_elements_set)
        value.element.num_of = layout_fields.num_of;

    if (param_element_size_set)
        value.element.size = layout_fields.size;

    if (param_element_alignment_set)
        value.element.alignment = layout_fields.alignment;

    if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
    {
        if ((value.ptr == NULL) && (value.element.num_of != 0))
            return ARCHI_STATUS_EVALUE;
        else if ((value.ptr != NULL) && (value.element.num_of == 0))
            return ARCHI_STATUS_EVALUE;
        else if ((value.element.alignment & (value.element.alignment - 1)) != 0)
            return ARCHI_STATUS_EVALUE;
    }

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *context_data = value;

    archi_reference_count_increment(value.ref_count);

    *context = context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_pointer_final)
{
    archi_reference_count_decrement(context->ref_count);
    free(context);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_pointer_get)
{
    if (strcmp("value", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = *context;
    }
    else if (strcmp("", slot.name) == 0)
    {
        if (slot.num_indices > 1)
            return ARCHI_STATUS_EMISUSE;
        else if (context->flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;

        if (context->element.size == 0)
            return ARCHI_STATUS_EMISUSE;

        ptrdiff_t offset = slot.index[0];
        if ((offset < 0) || ((size_t)offset >= context->element.num_of))
            return ARCHI_STATUS_EMISUSE;

        size_t padded_size = context->element.size;
        if (context->element.alignment != 0)
            padded_size = (padded_size + (context->element.alignment - 1)) &
                ~(context->element.alignment - 1);

        *value = (archi_pointer_t){
            .ptr = (char*)context->ptr + offset * padded_size,
                .ref_count = context->ref_count,
                .flags = context->flags,
                .element = {
                    .num_of = context->element.num_of - offset,
                    .size = context->element.size,
                    .alignment = context->element.alignment,
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

ARCHI_CONTEXT_SET_FUNC(archi_context_pointer_set)
{
    if (strcmp("value", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context->ref_count);

        *context = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_pointer_interface = {
    .init_fn = archi_context_pointer_init,
    .final_fn = archi_context_pointer_final,
    .get_fn = archi_context_pointer_get,
    .set_fn = archi_context_pointer_set,
};

