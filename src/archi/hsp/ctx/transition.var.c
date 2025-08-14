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
 * @brief Application context interface for hierarchical state processor transitions.
 */

#include "archi/hsp/ctx/transition.var.h"
#include "archi/hsp/api/transition.typ.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()
#include <stdbool.h>

struct archi_context_hsp_transition_data {
    archi_pointer_t transition;

    // References
    archi_pointer_t transition_function;
    archi_pointer_t transition_data;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_transition_init)
{
    archi_pointer_t transition_function = {0};
    archi_pointer_t transition_data = {0};

    bool param_function_set = false;
    bool param_data_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("function", params->name) == 0)
        {
            if (param_function_set)
                continue;
            param_function_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EVALUE;

            transition_function = params->value;
        }
        else if (strcmp("data", params->name) == 0)
        {
            if (param_data_set)
                continue;
            param_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            transition_data = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_hsp_transition_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_hsp_transition_t *transition = malloc(sizeof(*transition));
    if (transition == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *transition = (archi_hsp_transition_t){
        .function = (archi_hsp_transition_function_t)transition_function.fptr,
        .data = transition_data.ptr,
    };

    *context_data = (struct archi_context_hsp_transition_data){
        .transition = {
            .ptr = transition,
            .element = {
                .num_of = 1,
                .size = sizeof(*transition),
                .alignment = alignof(archi_hsp_transition_t),
            },
        },
        .transition_function = transition_function,
        .transition_data = transition_data,
    };

    archi_reference_count_increment(transition_function.ref_count);
    archi_reference_count_increment(transition_data.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_transition_final)
{
    struct archi_context_hsp_transition_data *context_data =
        (struct archi_context_hsp_transition_data*)context;

    archi_reference_count_decrement(context_data->transition_function.ref_count);
    archi_reference_count_decrement(context_data->transition_data.ref_count);
    free(context_data->transition.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_transition_get)
{
    struct archi_context_hsp_transition_data *context_data =
        (struct archi_context_hsp_transition_data*)context;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->transition_function;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->transition_data;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_transition_set)
{
    struct archi_context_hsp_transition_data *context_data =
        (struct archi_context_hsp_transition_data*)context;

    archi_hsp_transition_t *transition = context_data->transition.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->transition_function.ref_count);

        transition->function = (archi_hsp_transition_function_t)value.fptr;
        context_data->transition_function = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->transition_data.ref_count);

        transition->data = value.ptr;
        context_data->transition_data = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_hsp_transition_interface = {
    .init_fn = archi_context_hsp_transition_init,
    .final_fn = archi_context_hsp_transition_final,
    .get_fn = archi_context_hsp_transition_get,
    .set_fn = archi_context_hsp_transition_set,
};

