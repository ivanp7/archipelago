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
#include "archi/util/ref_count.fun.h"

#include <string.h> // for strcmp()

ARCHI_CONTEXT_INIT_FUNC(archi_context_pointer_init)
{
    archi_pointer_t value = {0};

    bool param_value_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("value", params->name) == 0)
        {
            if (param_value_set)
                continue;
            param_value_set = true;

            value = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_reference_count_increment(value.ref_count);

    context->public_value = context->private_value = value;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_pointer_final)
{
    archi_reference_count_decrement(context.private_value.ref_count);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_pointer_get)
{
    if (strcmp("value", slot.name) == 0)
    {
        if (slot.num_indices > 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.num_indices > 0) && (slot.index[0] >= context.public_value.element.num_of))
            return ARCHI_STATUS_EMISUSE;

        size_t offset = (slot.num_indices > 0) ? slot.index[0] : 0;
        size_t element_size = (context.public_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ?
            sizeof(archi_function_t) : context.public_value.element.size;

        *value = (archi_pointer_t){
            .ptr = (char*)context.public_value.ptr + offset * element_size,
            .ref_count = context.public_value.ref_count,
            .flags = context.public_value.flags,
            .element = {
                .num_of = context.public_value.element.num_of - offset,
                .size = context.public_value.element.size,
                .alignment = context.public_value.element.alignment,
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
        archi_reference_count_decrement(context.private_value.ref_count);

        struct archi_reference_count *ref_count = context.public_value.ref_count;

        context.public_value = context.private_value = value;
        context.public_value.ref_count = ref_count;
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

