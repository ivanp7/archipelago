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
 * @brief Context interface for arrays of pointers.
 */

#include "archi/ctx/interface/array.var.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

struct archi_context_array_data {
    archi_pointer_t array;
    archi_pointer_t *element;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_array_init)
{
    archi_pointer_flags_t flags = 0;
    size_t num_elements = 0;

    bool param_flags_set = false;
    bool param_num_elements_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("num_elements", params->name) == 0)
        {
            if (param_num_elements_set)
                continue;
            param_num_elements_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            num_elements = *(size_t*)params->value.ptr;
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
        else
            return ARCHI_STATUS_EKEY;
    }

    if (flags & ARCHI_POINTER_FLAG_FUNCTION)
        return ARCHI_STATUS_EMISUSE;

    struct archi_context_array_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *context_data = (struct archi_context_array_data){
        .array = {
            .flags = flags,
            .element = {
                .num_of = num_elements,
                .size = sizeof(void*),
                .alignment = alignof(void*),
            },
        },
    };

    if (num_elements > 0)
    {
        context_data->array.ptr = malloc(sizeof(void*) * num_elements);
        if (context_data->array.ptr == NULL)
        {
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        context_data->element = malloc(sizeof(archi_pointer_t) * num_elements);
        if (context_data->element == NULL)
        {
            free(context_data->array.ptr);
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        void **array = context_data->array.ptr;
        for (size_t i = 0; i < num_elements; i++)
        {
            array[i] = NULL;
            context_data->element[i] = (archi_pointer_t){0};
        }
    }

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_array_final)
{
    struct archi_context_array_data *context_data =
        (struct archi_context_array_data*)context;

    for (size_t i = 0; i < context_data->array.element.num_of; i++)
        archi_reference_count_decrement(context_data->element[i].ref_count);

    free(context_data->element);
    free(context_data->array.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_array_get)
{
    struct archi_context_array_data *context_data =
        (struct archi_context_array_data*)context;

    if (strcmp("", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= context_data->array.element.num_of))
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->element[slot.index[0]];
    }
    else if (strcmp("elements", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = context_data->element,
            .ref_count = context->ref_count,
            .element = {
                .num_of = context_data->array.element.num_of,
                .size = sizeof(*context_data->element),
                .alignment = alignof(archi_pointer_t),
            },
        };
    }
    else if (strcmp("num_elements", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->array.element.num_of,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->array.element.num_of),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_array_set)
{
    struct archi_context_array_data *context_data =
        (struct archi_context_array_data*)context;

    if (strcmp("", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= context_data->array.element.num_of))
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->element[slot.index[0]].ref_count);

        void **array = context_data->array.ptr;

        array[slot.index[0]] = value.ptr;
        context_data->element[slot.index[0]] = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_array_interface = {
    .init_fn = archi_context_array_init,
    .final_fn = archi_context_array_final,
    .get_fn = archi_context_array_get,
    .set_fn = archi_context_array_set,
};

