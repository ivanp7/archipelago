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
 * @brief Context interface for parameter lists.
 */

#include "archi/ctx/interface/parameters.var.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdbool.h>

ARCHI_CONTEXT_INIT_FUNC(archi_context_parameters_init)
{
    archi_pointer_t base = {0};

    bool param_base_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("base", params->name) == 0)
        {
            if (param_base_set)
                continue;
            param_base_set = true;

            base = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_reference_count_increment(base.ref_count);

    context->public_value = context->private_value = base;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_parameters_final)
{
    archi_context_parameter_list_t *node = context.public_value.ptr;

    while (node != context.private_value.ptr) // don't deallocate the base list (which is borrowed)
    {
        archi_context_parameter_list_t *next = node->next;

        archi_reference_count_decrement(node->value.ref_count);
        free(node);

        node = next;
    }

    archi_reference_count_decrement(context.private_value.ref_count);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_parameters_get)
{
    if (slot.num_indices != 0)
        return ARCHI_STATUS_EMISUSE;

    for (const archi_context_parameter_list_t *node = context.public_value.ptr;
            node != NULL; node = node->next)
    {
        if (strcmp(node->name, slot.name) == 0)
        {
            *value = node->value;
            return 0;
        }
    }

    return 1; // not found
}

ARCHI_CONTEXT_SET_FUNC(archi_context_parameters_set)
{
    if (slot.num_indices != 0)
        return ARCHI_STATUS_EMISUSE;

    archi_context_parameter_list_t *node = malloc(sizeof(*node));
    if (node == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *node = (archi_context_parameter_list_t){
        .next = context.public_value.ptr,
        .name = slot.name,
        .value = value,
    };

    archi_reference_count_increment(value.ref_count);

    context.public_value.ptr = node;
    context.public_value.element.num_of++; // increment the counter of nodes
    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_parameters_act)
{
    if (strcmp("_", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        for (; params != NULL; params = params->next)
        {
            archi_context_parameter_list_t *node = malloc(sizeof(*node));
            if (node == NULL)
                return ARCHI_STATUS_ENOMEMORY;

            *node = (archi_context_parameter_list_t){
                .next = context.public_value.ptr,
                .name = params->name,
                .value = params->value,
            };

            archi_reference_count_increment(params->value.ref_count);

            context.public_value.ptr = node;
            context.public_value.element.num_of++; // increment the counter of nodes
        }
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_parameters_interface = {
    .init_fn = archi_context_parameters_init,
    .final_fn = archi_context_parameters_final,
    .get_fn = archi_context_parameters_get,
    .set_fn = archi_context_parameters_set,
    .act_fn = archi_context_parameters_act,
};

