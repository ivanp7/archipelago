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
 * @brief Context interface for hierarchical state processor frames.
 */

#include "archi/hsp/ctx/frame.var.h"
#include "archi/hsp/api/state.fun.h"
#include "archi/hsp/api/exec.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

struct archi_context_data__hsp_frame_state_ref {
    archi_rcpointer_t function;
    archi_rcpointer_t data;
    archi_rcpointer_t metadata;
};

struct archi_context_data__hsp_frame {
    archi_rcpointer_t frame;

    // References
    struct archi_context_data__hsp_frame_state_ref *ref_state;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__hsp_frame)
{
    // Parse parameters
    size_t num_states = 0;
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "num_states", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            num_states = *(size_t*)parsed[index].value.ptr;
    }

    // Check validity of parameters
    if (num_states >= (ARCHI_POINTER_DATA_SIZE_MAX + 1) / sizeof(archi_rcpointer_t))
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "number of states is too big (%zu)", num_states);
        return NULL;
    }

    // Construct the context
    struct archi_context_data__hsp_frame *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_hsp_frame_t *hsp_frame = archi_hsp_frame_alloc(num_states);
    if (hsp_frame == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate frame [%zu]", num_states);
        return NULL;
    }

    *context_data = (struct archi_context_data__hsp_frame){
        .frame = {
            .ptr = hsp_frame,
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_hsp_frame_t),
        },
    };

    if (num_states != 0)
    {
        context_data->ref_state = malloc(sizeof(*context_data->ref_state) * num_states);
        if (context_data->ref_state == NULL)
        {
            free(hsp_frame);
            free(context_data);

            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to frame states [%zu]",
                    num_states);
            return NULL;
        }

        for (size_t i = 0; i < num_states; i++)
            context_data->ref_state[i] = (struct archi_context_data__hsp_frame_state_ref){0};
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__hsp_frame)
{
    struct archi_context_data__hsp_frame *context_data =
        (struct archi_context_data__hsp_frame*)context;

    archi_hsp_frame_t *hsp_frame = context_data->frame.ptr;

    for (size_t i = 0; i < hsp_frame->num_states; i++)
    {
        archi_rcpointer_disown(context_data->ref_state[i].function);
        archi_rcpointer_disown(context_data->ref_state[i].data);
        archi_rcpointer_disown(context_data->ref_state[i].metadata);
    }

    free(context_data->ref_state);
    free(context_data->frame.ptr);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__hsp_frame)
{
    struct archi_context_data__hsp_frame *context_data =
        (struct archi_context_data__hsp_frame*)context;

    archi_hsp_frame_t *hsp_frame = context_data->frame.ptr;

    if (!call)
    {
        if (ARCHI_STRING_COMPARE("num_states", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t num_states = hsp_frame->num_states;

            archi_rcpointer_t value = (archi_rcpointer_t){
                .ptr = &num_states,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("state.function", ==, slot.name))
        {
            if (slot.num_indices != 1)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
                return;
            }

            ptrdiff_t index = slot.index[0];
            if ((index < 0) || ((size_t)index >= hsp_frame->num_states))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu states)",
                        index, hsp_frame->num_states);
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_state[index].function);
        }
        else if (ARCHI_STRING_COMPARE("state.data", ==, slot.name))
        {
            if (slot.num_indices != 1)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
                return;
            }

            ptrdiff_t index = slot.index[0];
            if ((index < 0) || ((size_t)index >= hsp_frame->num_states))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu states)",
                        index, hsp_frame->num_states);
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_state[index].data);
        }
        else if (ARCHI_STRING_COMPARE("state.metadata", ==, slot.name))
        {
            if (slot.num_indices != 1)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
                return;
            }

            ptrdiff_t index = slot.index[0];
            if ((index < 0) || ((size_t)index >= hsp_frame->num_states))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu states)",
                        index, hsp_frame->num_states);
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_state[index].metadata);
        }
        else
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
    }
    else
    {
        if (ARCHI_STRING_COMPARE("execute", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            // Parse parameters
            archi_hsp_transition_t transition = {0};
            {
                archi_kvlist_parameter_t parsed[] = {
                    {.name = "transition", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_hsp_transition_t)},
                };

                if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                            ARCHI_ERROR_PARAMETER))
                    return;

                size_t index = 0;
                if (parsed[index].value_set)
                    transition = *(archi_hsp_transition_t*)parsed[index].value.ptr;
            }

            // Execute the frame
            archi_error_t error;

            ARCHI_ERROR_RESET_VAR(&error);
            archi_hsp_execute(hsp_frame, transition, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }
        else
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
            return;
        }

        ARCHI_ERROR_RESET();
    }
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__hsp_frame)
{
    struct archi_context_data__hsp_frame *context_data =
        (struct archi_context_data__hsp_frame*)context;

    archi_hsp_frame_t *hsp_frame = context_data->frame.ptr;

    if (ARCHI_STRING_COMPARE("state.function", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__function(ARCHI_POINTER_FUNCTION_TAG__HSP_STATE)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned function is not a HSP state function");
            return;
        }

        ptrdiff_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= hsp_frame->num_states))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu states)",
                    index, hsp_frame->num_states);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_state[index].function,
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        hsp_frame->state[index].function = (archi_hsp_state_function_t)value.fptr;
        context_data->ref_state[index].function = value;
    }
    else if (ARCHI_STRING_COMPARE("state.data", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not data");
            return;
        }

        ptrdiff_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= hsp_frame->num_states))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu states)",
                    index, hsp_frame->num_states);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_state[index].function,
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        hsp_frame->state[index].data = value.ptr;
        context_data->ref_state[index].data = value;
    }
    else if (ARCHI_STRING_COMPARE("state.metadata", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not data");
            return;
        }

        ptrdiff_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= hsp_frame->num_states))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu states)",
                    index, hsp_frame->num_states);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_state[index].function,
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        hsp_frame->state[index].metadata = value.ptr;
        context_data->ref_state[index].metadata = value;
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__hsp_frame = {
    .init_fn = archi_context_init__hsp_frame,
    .final_fn = archi_context_final__hsp_frame,
    .eval_fn = archi_context_eval__hsp_frame,
    .set_fn = archi_context_set__hsp_frame,
};

