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
 * @brief Application context interface for hierarchical state processor entities.
 */

#include "archi/builtin/hsp/context.var.h"
#include "archi/hsp/exec.fun.h"
#include "archi/hsp/instance.typ.h"
#include "archi/hsp/state/branch.typ.h"
#include "archi/util/ref_count.fun.h"

#include <stdlib.h> // for malloc(), realloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()
#include <stdbool.h>

#define REF_FUNCTION 0
#define REF_DATA 1
#define REF_METADATA 2
#define NUM_REFERENCES 3

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_state_init)
{
    archi_pointer_t hsp_state_function = {0};
    archi_pointer_t hsp_state_data = {0};
    archi_pointer_t hsp_state_metadata = {0};

    bool param_function_set = false;
    bool param_data_set = false;
    bool param_metadata_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("function", params->name) == 0)
        {
            if (param_function_set)
                continue;
            param_function_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EVALUE;

            hsp_state_function = params->value;
        }
        else if (strcmp("data", params->name) == 0)
        {
            if (param_data_set)
                continue;
            param_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            hsp_state_data = params->value;
        }
        else if (strcmp("metadata", params->name) == 0)
        {
            if (param_metadata_set)
                continue;
            param_metadata_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            hsp_state_metadata = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_hsp_state_t *hsp_state = malloc(sizeof(*hsp_state));
    if (hsp_state == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *hsp_state = (archi_hsp_state_t){
        .function = (archi_hsp_state_function_t)hsp_state_function.fptr,
        .data = hsp_state_data.ptr,
        .metadata = hsp_state_metadata.ptr,
    };

    context->num_references = NUM_REFERENCES;
    context->reference = malloc(sizeof(*context->reference) * context->num_references);
    if (context->reference == NULL)
    {
        free(hsp_state);
        return ARCHI_STATUS_ENOMEMORY;
    }

    context->reference[REF_FUNCTION] = hsp_state_function;
    context->reference[REF_DATA] = hsp_state_data;
    context->reference[REF_METADATA] = hsp_state_metadata;

    for (size_t i = 0; i < NUM_REFERENCES; i++)
        archi_reference_count_increment(context->reference[i].ref_count);

    context->public_value = (archi_pointer_t){
        .ptr = hsp_state,
        .element = {
            .num_of = 1,
            .size = sizeof(*hsp_state),
            .alignment = alignof(archi_hsp_state_t),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_state_final)
{
    for (size_t i = context.num_references; i-- > 0;)
        archi_reference_count_decrement(context.reference[i].ref_count);

    free(context.reference);
    free(context.public_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_state_get)
{
    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_FUNCTION];
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_DATA];
    }
    else if (strcmp("metadata", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_METADATA];
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_state_set)
{
    archi_hsp_state_t *hsp_state = context.public_value.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_FUNCTION].ref_count);

        hsp_state->function = (archi_hsp_state_function_t)value.fptr;
        context.reference[REF_FUNCTION] = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_DATA].ref_count);

        hsp_state->data = value.ptr;
        context.reference[REF_DATA] = value;
    }
    else if (strcmp("metadata", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_METADATA].ref_count);

        hsp_state->metadata = value.ptr;
        context.reference[REF_METADATA] = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_hsp_state_interface = {
    .init_fn = archi_context_hsp_state_init,
    .final_fn = archi_context_hsp_state_final,
    .get_fn = archi_context_hsp_state_get,
    .set_fn = archi_context_hsp_state_set,
};

#undef REF_FUNCTION
#undef REF_DATA
#undef REF_METADATA
#undef NUM_REFERENCES

/*****************************************************************************/

#define REF_FUNCTION 0
#define REF_DATA 1
#define NUM_REFERENCES 2

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_transition_init)
{
    archi_pointer_t hsp_transition_function = {0};
    archi_pointer_t hsp_transition_data = {0};

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

            hsp_transition_function = params->value;
        }
        else if (strcmp("data", params->name) == 0)
        {
            if (param_data_set)
                continue;
            param_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            hsp_transition_data = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_hsp_transition_t *hsp_transition = malloc(sizeof(*hsp_transition));
    if (hsp_transition == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *hsp_transition = (archi_hsp_transition_t){
        .function = (archi_hsp_transition_function_t)hsp_transition_function.fptr,
        .data = hsp_transition_data.ptr,
    };

    context->num_references = NUM_REFERENCES;
    context->reference = malloc(sizeof(*context->reference) * context->num_references);
    if (context->reference == NULL)
    {
        free(hsp_transition);
        return ARCHI_STATUS_ENOMEMORY;
    }

    context->reference[REF_FUNCTION] = hsp_transition_function;
    context->reference[REF_DATA] = hsp_transition_data;

    for (size_t i = 0; i < NUM_REFERENCES; i++)
        archi_reference_count_increment(context->reference[i].ref_count);

    context->public_value = (archi_pointer_t){
        .ptr = hsp_transition,
        .element = {
            .num_of = 1,
            .size = sizeof(*hsp_transition),
            .alignment = alignof(archi_hsp_transition_t),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_transition_final)
{
    for (size_t i = context.num_references; i-- > 0;)
        archi_reference_count_decrement(context.reference[i].ref_count);

    free(context.reference);
    free(context.public_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_transition_get)
{
    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_FUNCTION];
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_DATA];
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_transition_set)
{
    archi_hsp_transition_t *hsp_transition = context.public_value.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_FUNCTION].ref_count);

        hsp_transition->function = (archi_hsp_transition_function_t)value.fptr;
        context.reference[REF_FUNCTION] = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_DATA].ref_count);

        hsp_transition->data = value.ptr;
        context.reference[REF_DATA] = value;
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

#undef REF_FUNCTION
#undef REF_DATA
#undef NUM_REFERENCES

/*****************************************************************************/

#define REF_ENTRY_STATE 0
#define REF_TRANSITION 1
#define NUM_REFERENCES 2

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_init)
{
    archi_pointer_t hsp_entry_state = {0};
    archi_pointer_t hsp_transition = {0};

    bool param_entry_state_set = false;
    bool param_transition_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("entry_state", params->name) == 0)
        {
            if (param_entry_state_set)
                continue;
            param_entry_state_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            hsp_entry_state = params->value;
        }
        else if (strcmp("transition", params->name) == 0)
        {
            if (param_transition_set)
                continue;
            param_transition_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            hsp_transition = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_hsp_t *hsp = malloc(sizeof(*hsp));
    if (hsp == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *hsp = (archi_hsp_t){0};

    if (hsp_entry_state.ptr != NULL)
        hsp->entry_state = *(archi_hsp_state_t*)hsp_entry_state.ptr;

    if (hsp_transition.ptr != NULL)
        hsp->transition = *(archi_hsp_transition_t*)hsp_transition.ptr;

    context->num_references = NUM_REFERENCES;
    context->reference = malloc(sizeof(*context->reference) * context->num_references);
    if (context->reference == NULL)
    {
        free(hsp);
        return ARCHI_STATUS_ENOMEMORY;
    }

    context->reference[REF_ENTRY_STATE] = hsp_entry_state;
    context->reference[REF_TRANSITION] = hsp_transition;

    for (size_t i = 0; i < NUM_REFERENCES; i++)
        archi_reference_count_increment(context->reference[i].ref_count);

    context->public_value = (archi_pointer_t){
        .ptr = hsp,
        .element = {
            .num_of = 1,
            .size = sizeof(*hsp),
            .alignment = alignof(archi_hsp_t),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_final)
{
    for (size_t i = context.num_references; i-- > 0;)
        archi_reference_count_decrement(context.reference[i].ref_count);

    free(context.reference);
    free(context.public_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_get)
{
    if (strcmp("entry_state", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_ENTRY_STATE];
    }
    else if (strcmp("transition", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_TRANSITION];
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_set)
{
    archi_hsp_t *hsp = context.public_value.ptr;

    if (strcmp("entry_state", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_ENTRY_STATE].ref_count);

        if (value.ptr != NULL)
            hsp->entry_state = *(archi_hsp_state_t*)value.ptr;
        else
            hsp->entry_state = (archi_hsp_state_t){0};

        context.reference[REF_ENTRY_STATE] = value;
    }
    else if (strcmp("transition", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_TRANSITION].ref_count);

        if (value.ptr != NULL)
            hsp->transition = *(archi_hsp_transition_t*)value.ptr;
        else
            hsp->transition = (archi_hsp_transition_t){0};

        context.reference[REF_TRANSITION] = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_hsp_act)
{
    archi_hsp_t *hsp = context.public_value.ptr;

    if (strcmp("execute", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (params != NULL)
            return ARCHI_STATUS_EKEY;

        return archi_hsp_execute(*hsp);
    }
    else
        return ARCHI_STATUS_EKEY;
}

const archi_context_interface_t archi_context_hsp_interface = {
    .init_fn = archi_context_hsp_init,
    .final_fn = archi_context_hsp_final,
    .get_fn = archi_context_hsp_get,
    .set_fn = archi_context_hsp_set,
    .act_fn = archi_context_hsp_act,
};

#undef REF_ENTRY_STATE
#undef REF_TRANSITION
#undef NUM_REFERENCES

/*****************************************************************************/

#define REF_METADATA 0
#define NUM_REFERENCES 1

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_frame_init)
{
    size_t num_states = 0;
    archi_pointer_t hsp_frame_metadata = {0};

    bool param_num_states_set = false;
    bool param_metadata_set = false;

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
        else if (strcmp("metadata", params->name) == 0)
        {
            if (param_metadata_set)
                continue;
            param_metadata_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            hsp_frame_metadata = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_hsp_frame_t *hsp_frame = malloc(sizeof(*hsp_frame));
    if (hsp_frame == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *hsp_frame = (archi_hsp_frame_t){
        .num_states = num_states,
        .metadata = hsp_frame_metadata.ptr,
    };

    if (num_states > 0)
    {
        hsp_frame->state = malloc(sizeof(*hsp_frame->state) * num_states);
        if (hsp_frame->state == NULL)
        {
            free(hsp_frame);
            return ARCHI_STATUS_ENOMEMORY;
        }

        for (size_t i = 0; i < num_states; i++)
            hsp_frame->state[i] = (archi_hsp_state_t){0};
    }

    context->num_references = NUM_REFERENCES + num_states;
    context->reference = malloc(sizeof(*context->reference) * context->num_references);
    if (context->reference == NULL)
    {
        free(hsp_frame->state);
        free(hsp_frame);
        return ARCHI_STATUS_ENOMEMORY;
    }

    context->reference[REF_METADATA] = hsp_frame_metadata;

    for (size_t i = 0; i < NUM_REFERENCES; i++)
        archi_reference_count_increment(context->reference[i].ref_count);

    for (size_t i = NUM_REFERENCES; i < context->num_references; i++)
        context->reference[i] = (archi_pointer_t){0};

    context->public_value = (archi_pointer_t){
        .ptr = hsp_frame,
        .element = {
            .num_of = 1,
            .size = sizeof(*hsp_frame),
            .alignment = alignof(archi_hsp_frame_t),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_frame_final)
{
    for (size_t i = context.num_references; i-- > 0;)
        archi_reference_count_decrement(context.reference[i].ref_count);

    free(context.reference);

    archi_hsp_frame_t *hsp_frame = context.public_value.ptr;
    free(hsp_frame->state);
    free(hsp_frame);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_frame_get)
{
    archi_hsp_frame_t *hsp_frame = context.public_value.ptr;

    if (strcmp("num_states", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &hsp_frame->num_states,
            .ref_count = context.public_value.ref_count,
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
        else if (slot.index[0] >= hsp_frame->num_states)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[NUM_REFERENCES + slot.index[0]];
    }
    else if (strcmp("metadata", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_METADATA];
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_frame_set)
{
    archi_hsp_frame_t *hsp_frame = context.public_value.ptr;

    if (strcmp("num_states", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        size_t new_num_states = *(size_t*)value.ptr;

        // Reallocate the array of states
        if (new_num_states > 0)
        {
            archi_hsp_state_t *new_states =
                realloc(hsp_frame->state, sizeof(*new_states) * new_num_states);
            if (new_states == NULL)
                return ARCHI_STATUS_ENOMEMORY;

            hsp_frame->state = new_states;

            for (size_t i = hsp_frame->num_states; i < new_num_states; i++)
                hsp_frame->state[i] = (archi_hsp_state_t){0};
        }
        else
        {
            free(hsp_frame->state);
            hsp_frame->state = NULL;
        }

        // Reallocate the array of references
        {
            for (size_t i = new_num_states; i < hsp_frame->num_states; i++)
                archi_reference_count_decrement(context.reference[NUM_REFERENCES + i].ref_count);

            archi_pointer_t *new_reference =
                realloc(context.reference, sizeof(*new_reference) * (NUM_REFERENCES + new_num_states));
            if (new_reference == NULL)
                return ARCHI_STATUS_ENOMEMORY;

            context.reference = new_reference;

            for (size_t i = hsp_frame->num_states; i < new_num_states; i++)
                context.reference[NUM_REFERENCES + i] = (archi_pointer_t){0};
        }

        // Update the sizes of arrays
        hsp_frame->num_states = new_num_states;
        context.num_references = NUM_REFERENCES + new_num_states;
    }
    else if (strcmp("state", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if (slot.index[0] >= hsp_frame->num_states)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[NUM_REFERENCES + slot.index[0]].ref_count);

        if (value.ptr != NULL)
            hsp_frame->state[slot.index[0]] = *(archi_hsp_state_t*)value.ptr;
        else
            hsp_frame->state[slot.index[0]] = (archi_hsp_state_t){0};

        context.reference[NUM_REFERENCES + slot.index[0]] = value;
    }
    else if (strcmp("metadata", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_METADATA].ref_count);

        hsp_frame->metadata = value.ptr;
        context.reference[REF_METADATA] = value;
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
};

#undef REF_METADATA
#undef NUM_REFERENCES

/*****************************************************************************/

#define REF_SELECTOR_FN 0
#define REF_SELECTOR_DATA 1
#define NUM_REFERENCES 2

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_branch_state_data_init)
{
    size_t num_branches = 0;
    archi_pointer_t selector_fn = {0};
    archi_pointer_t selector_data = {0};

    bool param_num_branches_set = false;
    bool param_selector_fn_set = false;
    bool param_selector_data_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("num_branches", params->name) == 0)
        {
            if (param_num_branches_set)
                continue;
            param_num_branches_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            num_branches = *(size_t*)params->value.ptr;
        }
        else if (strcmp("selector_fn", params->name) == 0)
        {
            if (param_selector_fn_set)
                continue;
            param_selector_fn_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EVALUE;

            selector_fn = params->value;
        }
        else if (strcmp("selector_data", params->name) == 0)
        {
            if (param_selector_data_set)
                continue;
            param_selector_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            selector_data = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_hsp_branch_state_data_t *branch_state_data = malloc(sizeof(*branch_state_data));
    if (branch_state_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *branch_state_data = (archi_hsp_branch_state_data_t){
        .num_branches = num_branches,
        .selector_fn = (archi_hsp_branch_selector_func_t)selector_fn.fptr,
        .selector_data = selector_data.ptr,
    };

    if (num_branches > 0)
    {
        branch_state_data->branch = malloc(sizeof(*branch_state_data->branch) * num_branches);
        if (branch_state_data->branch == NULL)
        {
            free(branch_state_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        for (size_t i = 0; i < num_branches; i++)
            branch_state_data->branch[i] = (archi_hsp_frame_t){0};
    }

    context->num_references = NUM_REFERENCES + num_branches;
    context->reference = malloc(sizeof(*context->reference) * context->num_references);
    if (context->reference == NULL)
    {
        free(branch_state_data->branch);
        free(branch_state_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    context->reference[REF_SELECTOR_FN] = selector_fn;
    context->reference[REF_SELECTOR_DATA] = selector_data;

    for (size_t i = 0; i < NUM_REFERENCES; i++)
        archi_reference_count_increment(context->reference[i].ref_count);

    for (size_t i = NUM_REFERENCES; i < context->num_references; i++)
        context->reference[i] = (archi_pointer_t){0};

    context->public_value = (archi_pointer_t){
        .ptr = branch_state_data,
        .element = {
            .num_of = 1,
            .size = sizeof(*branch_state_data),
            .alignment = alignof(archi_hsp_branch_state_data_t),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_branch_state_data_final)
{
    for (size_t i = context.num_references; i-- > 0;)
        archi_reference_count_decrement(context.reference[i].ref_count);

    free(context.reference);

    archi_hsp_branch_state_data_t *branch_state_data = context.public_value.ptr;
    free(branch_state_data->branch);
    free(branch_state_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_branch_state_data_get)
{
    archi_hsp_branch_state_data_t *branch_state_data = context.public_value.ptr;

    if (strcmp("num_branches", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &branch_state_data->num_branches,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(branch_state_data->num_branches),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("branch", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if (slot.index[0] >= branch_state_data->num_branches)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[NUM_REFERENCES + slot.index[0]];
    }
    else if (strcmp("selector_fn", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_SELECTOR_FN];
    }
    else if (strcmp("selector_data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_SELECTOR_DATA];
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_branch_state_data_set)
{
    archi_hsp_branch_state_data_t *branch_state_data = context.public_value.ptr;

    if (strcmp("num_branches", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        size_t new_num_branches = *(size_t*)value.ptr;

        // Reallocate the array of states
        if (new_num_branches > 0)
        {
            archi_hsp_frame_t *new_branches =
                realloc(branch_state_data->branch, sizeof(*new_branches) * new_num_branches);
            if (new_branches == NULL)
                return ARCHI_STATUS_ENOMEMORY;

            branch_state_data->branch = new_branches;

            for (size_t i = branch_state_data->num_branches; i < new_num_branches; i++)
                branch_state_data->branch[i] = (archi_hsp_frame_t){0};
        }
        else
        {
            free(branch_state_data->branch);
            branch_state_data->branch = NULL;
        }

        // Reallocate the array of references
        {
            for (size_t i = new_num_branches; i < branch_state_data->num_branches; i++)
                archi_reference_count_decrement(context.reference[NUM_REFERENCES + i].ref_count);

            archi_pointer_t *new_reference =
                realloc(context.reference, sizeof(*new_reference) * (NUM_REFERENCES + new_num_branches));
            if (new_reference == NULL)
                return ARCHI_STATUS_ENOMEMORY;

            context.reference = new_reference;

            for (size_t i = branch_state_data->num_branches; i < new_num_branches; i++)
                context.reference[NUM_REFERENCES + i] = (archi_pointer_t){0};
        }

        // Update the sizes of arrays
        branch_state_data->num_branches = new_num_branches;
        context.num_references = NUM_REFERENCES + new_num_branches;
    }
    else if (strcmp("branch", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if (slot.index[0] >= branch_state_data->num_branches)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[NUM_REFERENCES + slot.index[0]].ref_count);

        if (value.ptr != NULL)
            branch_state_data->branch[slot.index[0]] = *(archi_hsp_frame_t*)value.ptr;
        else
            branch_state_data->branch[slot.index[0]] = (archi_hsp_frame_t){0};

        context.reference[NUM_REFERENCES + slot.index[0]] = value;
    }
    else if (strcmp("selector_fn", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_SELECTOR_FN].ref_count);

        branch_state_data->selector_fn = (archi_hsp_branch_selector_func_t)value.fptr;
        context.reference[REF_SELECTOR_FN] = value;
    }
    else if (strcmp("selector_data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_SELECTOR_DATA].ref_count);

        branch_state_data->selector_data = value.ptr;
        context.reference[REF_SELECTOR_DATA] = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_hsp_branch_state_data_interface = {
    .init_fn = archi_context_hsp_branch_state_data_init,
    .final_fn = archi_context_hsp_branch_state_data_final,
    .get_fn = archi_context_hsp_branch_state_data_get,
    .set_fn = archi_context_hsp_branch_state_data_set,
};

#undef REF_SELECTOR_FN
#undef REF_SELECTOR_DATA
#undef NUM_REFERENCES

