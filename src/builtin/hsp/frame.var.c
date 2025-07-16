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
 * @brief Application context interface for hierarchical state processor frames.
 */

#include "archi/builtin/hsp/frame.var.h"
#include "archi/hsp/state.fun.h"
#include "archi/hsp/exec.fun.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()
#include <stdbool.h>

struct archi_context_hsp_frame_data {
    archi_pointer_t frame;

    // References
    archi_pointer_t *frame_state_function;
    archi_pointer_t *frame_state_data;
    archi_pointer_t *frame_state_metadata;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_frame_init)
{
    size_t num_states = 0;

    bool param_num_states_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("num_states", params->name) == 0)
        {
            if (param_num_states_set)
                continue;
            param_num_states_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            num_states = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_hsp_frame_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_hsp_frame_t *hsp_frame = archi_hsp_frame_alloc(num_states);
    if (hsp_frame == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *context_data = (struct archi_context_hsp_frame_data){
        .frame = {
            .ptr = hsp_frame,
            .element = {
                .num_of = 1,
                .size = ARCHI_SIZEOF_FLEXIBLE(archi_hsp_frame_t, state, num_states),
                .alignment = alignof(archi_hsp_frame_t),
            },
        },
    };

    if (num_states > 0)
    {
        context_data->frame_state_function = malloc(sizeof(*context_data->frame_state_function) * num_states);
        if (context_data->frame_state_function == NULL)
        {
            free(hsp_frame);
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        context_data->frame_state_data = malloc(sizeof(*context_data->frame_state_data) * num_states);
        if (context_data->frame_state_data == NULL)
        {
            free(context_data->frame_state_function);
            free(hsp_frame);
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        context_data->frame_state_metadata = malloc(sizeof(*context_data->frame_state_metadata) * num_states);
        if (context_data->frame_state_metadata == NULL)
        {
            free(context_data->frame_state_data);
            free(context_data->frame_state_function);
            free(hsp_frame);
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        for (size_t i = 0; i < num_states; i++)
        {
            context_data->frame_state_function[i] = (archi_pointer_t){0};
            context_data->frame_state_data[i] = (archi_pointer_t){0};
            context_data->frame_state_metadata[i] = (archi_pointer_t){0};
        }
    }

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_frame_final)
{
    struct archi_context_hsp_frame_data *context_data =
        (struct archi_context_hsp_frame_data*)context;

    archi_hsp_frame_t *hsp_frame = context_data->frame.ptr;

    for (size_t i = 0; i < hsp_frame->num_states; i++)
    {
        archi_reference_count_decrement(context_data->frame_state_function[i].ref_count);
        archi_reference_count_decrement(context_data->frame_state_data[i].ref_count);
        archi_reference_count_decrement(context_data->frame_state_metadata[i].ref_count);
    }

    free(context_data->frame_state_function);
    free(context_data->frame_state_data);
    free(context_data->frame_state_metadata);
    free(hsp_frame);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_frame_get)
{
    struct archi_context_hsp_frame_data *context_data =
        (struct archi_context_hsp_frame_data*)context;

    archi_hsp_frame_t *hsp_frame = context_data->frame.ptr;

    if (strcmp("num_states", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = (void*)&hsp_frame->num_states,
            .ref_count = context_data->frame.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(hsp_frame->num_states),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("state", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= hsp_frame->num_states))
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &hsp_frame->state[slot.index[0]],
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(hsp_frame->state[0]),
                .alignment = alignof(archi_hsp_state_t),
            },
        };
    }
    else if (strcmp("state.function", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= hsp_frame->num_states))
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->frame_state_function[slot.index[0]];
    }
    else if (strcmp("state.data", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= hsp_frame->num_states))
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->frame_state_data[slot.index[0]];
    }
    else if (strcmp("state.metadata", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= hsp_frame->num_states))
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->frame_state_metadata[slot.index[0]];
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_frame_set)
{
    struct archi_context_hsp_frame_data *context_data =
        (struct archi_context_hsp_frame_data*)context;

    archi_hsp_frame_t *hsp_frame = context_data->frame.ptr;

    if (strcmp("state.function", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= hsp_frame->num_states))
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->frame_state_function[slot.index[0]].ref_count);

        hsp_frame->state[slot.index[0]].function = (archi_hsp_state_function_t)value.fptr;
        context_data->frame_state_function[slot.index[0]] = value;
    }
    else if (strcmp("state.data", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= hsp_frame->num_states))
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->frame_state_data[slot.index[0]].ref_count);

        hsp_frame->state[slot.index[0]].data = value.ptr;
        context_data->frame_state_data[slot.index[0]] = value;
    }
    else if (strcmp("state.metadata", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.num_indices > 0) && ((slot.index[0] < 0) || ((size_t)slot.index[0] >= hsp_frame->num_states)))
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->frame_state_metadata[slot.index[0]].ref_count);

        hsp_frame->state[slot.index[0]].metadata = value.ptr;
        context_data->frame_state_metadata[slot.index[0]] = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_hsp_frame_act)
{
    struct archi_context_hsp_frame_data *context_data =
        (struct archi_context_hsp_frame_data*)context;

    archi_hsp_frame_t *hsp_frame = context_data->frame.ptr;

    if (strcmp("execute", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_hsp_transition_t transition = {0};

        bool param_transition_function_set = false;
        bool param_transition_data_set = false;

        for (; params != NULL; params = params->next)
        {
            if (strcmp("transition_function", params->name) == 0)
            {
                if (param_transition_function_set)
                    continue;
                param_transition_function_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                    return ARCHI_STATUS_EVALUE;

                transition.function = (archi_hsp_transition_function_t)params->value.fptr;
            }
            else if (strcmp("transition_data", params->name) == 0)
            {
                if (param_transition_data_set)
                    continue;
                param_transition_data_set = true;

                if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                    return ARCHI_STATUS_EVALUE;

                transition.data = params->value.ptr;
            }
            else
                return ARCHI_STATUS_EKEY;
        }

        archi_status_t code = archi_hsp_execute(hsp_frame, transition);
        if (code != 0)
            return code;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_hsp_frame_interface = {
    .init_fn = archi_context_hsp_frame_init,
    .final_fn = archi_context_hsp_frame_final,
    .get_fn = archi_context_hsp_frame_get,
    .set_fn = archi_context_hsp_frame_set,
    .act_fn = archi_context_hsp_frame_act,
};

