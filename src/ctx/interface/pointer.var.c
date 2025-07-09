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
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), memmove()
#include <stdalign.h> // for alignof()

struct archi_context_pointer_data {
    archi_pointer_t pointer;
    archi_reference_count_t ref_count;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_pointer_init)
{
    archi_pointer_t value = {0};
    archi_pointer_flags_t flags = 0;
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

            flags = *(archi_pointer_flags_t*)params->value.ptr;
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

    struct archi_context_pointer_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *context_data = (struct archi_context_pointer_data){
        .pointer = {
            .ptr = value.ptr,
            .flags = value.flags,
            .element = value.element,
        },
        .ref_count = value.ref_count,
    };

    archi_reference_count_increment(value.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_pointer_final)
{
    struct archi_context_pointer_data *context_data =
        (struct archi_context_pointer_data*)context;

    archi_reference_count_decrement(context_data->ref_count);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_pointer_get)
{
    struct archi_context_pointer_data *context_data =
        (struct archi_context_pointer_data*)context;

    if (strcmp("", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if (context_data->pointer.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;
        else if (context_data->pointer.element.size == 0)
            return ARCHI_STATUS_EMISUSE;

        ptrdiff_t offset = slot.index[0];
        if ((offset < 0) || ((size_t)offset >= context_data->pointer.element.num_of))
            return ARCHI_STATUS_EMISUSE;

        size_t padded_size = context_data->pointer.element.size;
        if (context_data->pointer.element.alignment != 0)
            padded_size = ARCHI_SIZE_PADDED(padded_size, context_data->pointer.element.alignment);

        *value = (archi_pointer_t){
            .ptr = (char*)context_data->pointer.ptr + offset * padded_size,
                .ref_count = context_data->ref_count,
                .flags = context_data->pointer.flags,
                .element = {
                    .num_of = context_data->pointer.element.num_of - offset,
                    .size = context_data->pointer.element.size,
                    .alignment = context_data->pointer.element.alignment,
                },
        };
    }
    else if (strcmp("flags", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->pointer.flags,
            .ref_count = context_data->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->pointer.flags),
                .alignment = alignof(archi_pointer_flags_t),
            },
        };
    }
    else if (strcmp("layout", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->pointer.element,
            .ref_count = context_data->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->pointer.element),
                .alignment = alignof(archi_array_layout_t),
            },
        };
    }
    else if (strcmp("num_elements", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->pointer.element.num_of,
            .ref_count = context_data->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->pointer.element.num_of),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->pointer.element.size,
            .ref_count = context_data->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->pointer.element.size),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("element_alignment", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->pointer.element.alignment,
            .ref_count = context_data->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->pointer.element.alignment),
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
    struct archi_context_pointer_data *context_data =
        (struct archi_context_pointer_data*)context;

    if (strcmp("value", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->ref_count);

        archi_reference_count_t ref_count = context_data->pointer.ref_count;

        context_data->pointer = (archi_pointer_t){
            .ptr = value.ptr,
            .ref_count = ref_count,
            .flags = value.flags,
            .element = value.element,
        };
        context_data->ref_count = value.ref_count;
    }
    else if (strcmp("", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if (context_data->pointer.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;
        else if ((context_data->pointer.flags & ARCHI_POINTER_FLAG_WRITABLE) == 0)
            return ARCHI_STATUS_EMISUSE;
        else if (context_data->pointer.element.size == 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;
        else if (value.ptr == NULL)
            return ARCHI_STATUS_EMISUSE;
        else if (value.element.size != context_data->pointer.element.size)
            return ARCHI_STATUS_EMISUSE;

        ptrdiff_t offset = slot.index[0];
        if ((offset < 0) || ((size_t)offset >= context_data->pointer.element.num_of))
            return ARCHI_STATUS_EMISUSE;

        size_t padded_size = context_data->pointer.element.size;
        if (context_data->pointer.element.alignment != 0)
            padded_size = ARCHI_SIZE_PADDED(padded_size, context_data->pointer.element.alignment);

        memmove((char*)context_data->pointer.ptr + offset * padded_size, value.ptr, value.element.size);
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_pointer_act)
{
    struct archi_context_pointer_data *context_data =
        (struct archi_context_pointer_data*)context;

    if (strcmp("update", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_pointer_t value = context_data->pointer;
        value.ref_count = context_data->ref_count;

        archi_pointer_flags_t flags = 0;
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

                flags = *(archi_pointer_flags_t*)params->value.ptr;
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

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->ref_count);

        archi_reference_count_t ref_count = context_data->pointer.ref_count;

        context_data->pointer = (archi_pointer_t){
            .ptr = value.ptr,
            .ref_count = ref_count,
            .flags = value.flags,
            .element = value.element,
        };
        context_data->ref_count = value.ref_count;
    }
    else if (strcmp("copy", action.name) == 0)
    {
        if (action.num_indices > 1)
            return ARCHI_STATUS_EMISUSE;
        else if (context_data->pointer.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;
        else if (context_data->pointer.ptr == NULL)
            return ARCHI_STATUS_EMISUSE;
        else if (context_data->pointer.element.size == 0)
            return ARCHI_STATUS_EMISUSE;

        ptrdiff_t offset = (action.num_indices > 0) ? action.index[0] : 0;
        if ((offset < 0) || ((size_t)offset >= context_data->pointer.element.num_of))
            return ARCHI_STATUS_EMISUSE;

        archi_pointer_t source = {0};
        size_t source_offset = 0;
        size_t num_elements = 0;

        bool param_source_set = false;
        bool param_source_offset_set = false;
        bool param_num_elements_set = false;

        for (; params != NULL; params = params->next)
        {
            if (strcmp("source", params->name) == 0)
            {
                if (param_source_set)
                    continue;
                param_source_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                source = params->value;
            }
            else if (strcmp("source_offset", params->name) == 0)
            {
                if (param_source_offset_set)
                    continue;
                param_source_offset_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                source_offset = *(size_t*)params->value.ptr;
            }
            else if (strcmp("num_elements", params->name) == 0)
            {
                if (param_num_elements_set)
                    continue;
                param_num_elements_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                num_elements = *(size_t*)params->value.ptr;
            }
            else
                return ARCHI_STATUS_EKEY;
        }

        if (source.element.size != context_data->pointer.element.size)
            return ARCHI_STATUS_EMISUSE;

        size_t padded_size = context_data->pointer.element.size;
        if (context_data->pointer.element.alignment != 0)
            padded_size = ARCHI_SIZE_PADDED(padded_size, context_data->pointer.element.alignment);

        {
            size_t src_padded_size = source.element.size;
            if (source.element.alignment != 0)
                src_padded_size = ARCHI_SIZE_PADDED(src_padded_size, source.element.alignment);

            if (padded_size != src_padded_size)
                return ARCHI_STATUS_EMISUSE;
        }

        if (!param_num_elements_set)
            num_elements = context_data->pointer.element.num_of - offset;

        if (source_offset >= source.element.num_of)
            return ARCHI_STATUS_EMISUSE;
        else if (num_elements > source.element.num_of - source_offset)
            return ARCHI_STATUS_EMISUSE;

        memmove((char*)context_data->pointer.ptr + offset * padded_size,
            (char*)source.ptr + source_offset * padded_size,
            num_elements * padded_size);
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
    .act_fn = archi_context_pointer_act,
};

