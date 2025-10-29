/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
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
 * @brief Context interface for HSP array transition data.
 */

#include "archi/hsp/hsp-ctx/array_transition_data.var.h"
#include "archi/hsp/hsp/array_transition.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

struct archi_context_data__hsp_transition_data__array {
    archi_rcpointer_t array;
    archi_rcpointer_t *ref_transition;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__hsp_transition_data__array)
{
    // Parse parameters
    size_t num_transitions = 0;
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "num_transitions", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            num_transitions = *(size_t*)parsed[index].value.ptr;
    }

    // Check validity of parameters
    if (num_transitions >= (ARCHI_POINTER_DATA_SIZE_MAX + 1) / sizeof(archi_rcpointer_t))
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "number of transitions is too big (%zu)", num_transitions);
        return NULL;
    }

    // Construct the context
    struct archi_context_data__hsp_transition_data__array *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_hsp_transition_data__array_t *hsp_transition_array = archi_hsp_transition_data_alloc__array(num_transitions);
    if (hsp_transition_array == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of transitions [%zu]", num_transitions);
        return NULL;
    }

    *context_data = (struct archi_context_data__hsp_transition_data__array){
        .array = {
            .ptr = hsp_transition_array,
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_hsp_transition_data__array_t),
        },
    };

    if (num_transitions != 0)
    {
        context_data->ref_transition = malloc(sizeof(*context_data->ref_transition) * num_transitions);
        if (context_data->ref_transition == NULL)
        {
            free(hsp_transition_array);
            free(context_data);

            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to transitions [%zu]",
                    num_transitions);
            return NULL;
        }

        for (size_t i = 0; i < num_transitions; i++)
            context_data->ref_transition[i] = (archi_rcpointer_t){0};
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__hsp_transition_data__array)
{
    struct archi_context_data__hsp_transition_data__array *context_data =
        (struct archi_context_data__hsp_transition_data__array*)context;

    archi_hsp_transition_data__array_t *hsp_transition_array = context_data->array.ptr;

    for (size_t i = 0; i < hsp_transition_array->num_transitions; i++)
        archi_rcpointer_disown(context_data->ref_transition[i]);

    free(context_data->ref_transition);
    free(context_data->array.ptr);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__hsp_transition_data__array)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__hsp_transition_data__array *context_data =
        (struct archi_context_data__hsp_transition_data__array*)context;

    archi_hsp_transition_data__array_t *hsp_transition_array = context_data->array.ptr;

    if (ARCHI_STRING_COMPARE("num_transitions", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t num_transitions = hsp_transition_array->num_transitions;

        archi_rcpointer_t value = (archi_rcpointer_t){
            .ptr = &num_transitions,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("transition", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }

        ptrdiff_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= hsp_transition_array->num_transitions))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu transitions)",
                    index, hsp_transition_array->num_transitions);
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->ref_transition[index]);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__hsp_transition_data__array)
{
    struct archi_context_data__hsp_transition_data__array *context_data =
        (struct archi_context_data__hsp_transition_data__array*)context;

    archi_hsp_transition_data__array_t *hsp_transition_array = context_data->array.ptr;

    if (ARCHI_STRING_COMPARE("transition", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(value.ptr != NULL, archi_hsp_transition_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a HSP transition");
            return;
        }
        else if (ARCHI_POINTER_TO_STACK(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "HSP transition is on the stack");
            return;
        }

        ptrdiff_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= hsp_transition_array->num_transitions))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu transitions)",
                    index, hsp_transition_array->num_transitions);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_transition[index],
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        hsp_transition_array->transition[index] = value.ptr;
        context_data->ref_transition[index] = value;
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__hsp_transition_data__array = {
    .init_fn = archi_context_init__hsp_transition_data__array,
    .final_fn = archi_context_final__hsp_transition_data__array,
    .eval_fn = archi_context_eval__hsp_transition_data__array,
    .set_fn = archi_context_set__hsp_transition_data__array,
};

