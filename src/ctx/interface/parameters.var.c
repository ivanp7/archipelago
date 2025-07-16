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
#include "archi/util/alloc.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdbool.h>
#include <stdalign.h>

static
archi_status_t
archi_context_parameters_copy(
        archi_pointer_t *base,
        const archi_parameter_list_t *params)
{
    archi_parameter_list_t *head = NULL, *tail = NULL;

    for (; params != NULL; params = params->next)
    {
        archi_parameter_list_t *node = malloc(sizeof(*node));
        if (node == NULL)
            goto failure;

        char *name = archi_copy_string(params->name);
        if (name == NULL)
        {
            free(node);
            goto failure;
        }

        *node = (archi_parameter_list_t){
            .name = name,
            .value = params->value,
        };

        if (tail != NULL)
        {
            tail->next = node;
            tail = node;
        }
        else
            head = tail = node;
    }

    for (params = head; params != NULL; params = params->next)
    {
        archi_reference_count_increment(params->value.ref_count);
        base->flags++; // increment the counter of nodes
    }

    if (tail != NULL)
        tail->next = base->ptr;
    base->ptr = head;

    return 0;

failure:
    while (head != NULL)
    {
        archi_parameter_list_t *next = head->next;

        free((char*)head->name);
        free(head);

        head = next;
    }

    return ARCHI_STATUS_ENOMEMORY;
}

ARCHI_CONTEXT_INIT_FUNC(archi_context_parameters_init)
{
    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *context_data = (archi_pointer_t){
        .element = {
            .num_of = 1,
            .size = sizeof(archi_parameter_list_t),
            .alignment = alignof(archi_parameter_list_t),
        },
    };

    archi_status_t code = archi_context_parameters_copy(context_data, params);
    if (code != 0)
    {
        free(context_data);
        return code;
    }

    *context = context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_parameters_final)
{
    archi_parameter_list_t *node = context->ptr;

    while (node != NULL)
    {
        archi_parameter_list_t *next = node->next;

        archi_reference_count_decrement(node->value.ref_count);
        free((char*)node->name);
        free(node);

        node = next;
    }

    free(context);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_parameters_get)
{
    if (slot.num_indices != 0)
        return ARCHI_STATUS_EMISUSE;

    for (archi_parameter_list_t *node = context->ptr;
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

    for (archi_parameter_list_t *node = context->ptr;
            node != NULL; node = node->next)
    {
        if (strcmp(node->name, slot.name) == 0)
        {
            archi_reference_count_increment(value.ref_count);
            archi_reference_count_decrement(node->value.ref_count);

            node->value = value;
            return 0;
        }
    }

    archi_parameter_list_t *node = malloc(sizeof(*node));
    if (node == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    char *name = archi_copy_string(slot.name);
    if (name == NULL)
    {
        free(node);
        return ARCHI_STATUS_ENOMEMORY;
    }

    archi_reference_count_increment(value.ref_count);

    *node = (archi_parameter_list_t){
        .next = context->ptr,
        .name = name,
        .value = value,
    };

    context->ptr = node;
    context->flags++; // increment the counter of nodes

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_parameters_act)
{
    if (strcmp("_", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_status_t code = archi_context_parameters_copy(context, params);
        if (code != 0)
            return code;
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

