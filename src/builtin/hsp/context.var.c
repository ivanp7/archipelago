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
#include "archi/hsp/state/branch.typ.h"
#include "archi/util/alloc.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()
#include <stdbool.h>

struct archi_context_hsp_state_data {
    archi_pointer_t state;

    archi_pointer_t state_function;
    archi_pointer_t state_data;
    archi_pointer_t state_metadata;
};

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

    struct archi_context_hsp_state_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_hsp_state_t *hsp_state = malloc(sizeof(*hsp_state));
    if (hsp_state == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *hsp_state = (archi_hsp_state_t){
        .function = (archi_hsp_state_function_t)hsp_state_function.fptr,
        .data = hsp_state_data.ptr,
        .metadata = hsp_state_metadata.ptr,
    };

    *context_data = (struct archi_context_hsp_state_data){
        .state = {
            .ptr = hsp_state,
            .element = {
                .num_of = 1,
                .size = sizeof(*hsp_state),
                .alignment = alignof(archi_hsp_state_t),
            },
        },
        .state_function = hsp_state_function,
        .state_data = hsp_state_data,
        .state_metadata = hsp_state_metadata,
    };

    archi_reference_count_increment(hsp_state_function.ref_count);
    archi_reference_count_increment(hsp_state_data.ref_count);
    archi_reference_count_increment(hsp_state_metadata.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_state_final)
{
    struct archi_context_hsp_state_data *context_data =
        (struct archi_context_hsp_state_data*)context;

    archi_reference_count_decrement(context_data->state_function.ref_count);
    archi_reference_count_decrement(context_data->state_data.ref_count);
    archi_reference_count_decrement(context_data->state_metadata.ref_count);
    free(context_data->state.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_state_get)
{
    struct archi_context_hsp_state_data *context_data =
        (struct archi_context_hsp_state_data*)context;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->state_function;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->state_data;
    }
    else if (strcmp("metadata", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->state_metadata;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_state_set)
{
    struct archi_context_hsp_state_data *context_data =
        (struct archi_context_hsp_state_data*)context;

    archi_hsp_state_t *hsp_state = context_data->state.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->state_function.ref_count);

        hsp_state->function = (archi_hsp_state_function_t)value.fptr;
        context_data->state_function = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->state_data.ref_count);

        hsp_state->data = value.ptr;
        context_data->state_data = value;
    }
    else if (strcmp("metadata", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->state_metadata.ref_count);

        hsp_state->metadata = value.ptr;
        context_data->state_metadata = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_hsp_state_act)
{
    struct archi_context_hsp_state_data *context_data =
        (struct archi_context_hsp_state_data*)context;

    archi_hsp_state_t *hsp_state = context_data->state.ptr;

    if (strcmp("execute", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_hsp_transition_t hsp_transition = {0};

        bool param_transition_set = false;

        for (; params != NULL; params = params->next)
        {
            if (strcmp("transition", params->name) == 0)
            {
                if (param_transition_set)
                    continue;
                param_transition_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                hsp_transition = *(archi_hsp_transition_t*)params->value.ptr;
            }
            else
                return ARCHI_STATUS_EKEY;
        }

        archi_status_t code = archi_hsp_execute(*hsp_state, hsp_transition);
        if (code != 0)
            return code;
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
    .act_fn = archi_context_hsp_state_act,
};

/*****************************************************************************/

struct archi_context_hsp_transition_data {
    archi_pointer_t transition;

    archi_pointer_t transition_function;
    archi_pointer_t transition_data;
};

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

    struct archi_context_hsp_transition_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_hsp_transition_t *hsp_transition = malloc(sizeof(*hsp_transition));
    if (hsp_transition == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *hsp_transition = (archi_hsp_transition_t){
        .function = (archi_hsp_transition_function_t)hsp_transition_function.fptr,
        .data = hsp_transition_data.ptr,
    };

    *context_data = (struct archi_context_hsp_transition_data){
        .transition = {
            .ptr = hsp_transition,
            .element = {
                .num_of = 1,
                .size = sizeof(*hsp_transition),
                .alignment = alignof(archi_hsp_transition_t),
            },
        },
        .transition_function = hsp_transition_function,
        .transition_data = hsp_transition_data,
    };

    archi_reference_count_increment(hsp_transition_function.ref_count);
    archi_reference_count_increment(hsp_transition_data.ref_count);

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

    archi_hsp_transition_t *hsp_transition = context_data->transition.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->transition_function.ref_count);

        hsp_transition->function = (archi_hsp_transition_function_t)value.fptr;
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

        hsp_transition->data = value.ptr;
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

/*****************************************************************************/

struct archi_context_hsp_frame_data {
    archi_pointer_t frame;

    archi_pointer_t frame_metadata;
    archi_pointer_t *frame_state;
};

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

    struct archi_context_hsp_frame_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_hsp_frame_t *hsp_frame = malloc(sizeof(*hsp_frame));
    if (hsp_frame == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *hsp_frame = (archi_hsp_frame_t){
        .num_states = num_states,
        .metadata = hsp_frame_metadata.ptr,
    };

    *context_data = (struct archi_context_hsp_frame_data){
        .frame = {
            .ptr = hsp_frame,
            .element = {
                .num_of = 1,
                .size = sizeof(*hsp_frame),
                .alignment = alignof(archi_hsp_frame_t),
            },
        },
        .frame_metadata = hsp_frame_metadata,
    };

    {
        archi_pointer_t state_array = {
            .ptr = hsp_frame->state,
            .element = {
                .size = sizeof(*hsp_frame->state),
                .alignment = alignof(archi_hsp_state_t),
            },
        };

        archi_hsp_state_t null_state = {0};

        archi_status_t code = archi_resize_array(&state_array,
                &context_data->frame_state, num_states, &null_state);
        if (code != 0)
        {
            free(hsp_frame);
            free(context_data);
            return code;
        }

        hsp_frame->state = state_array.ptr;
    }

    archi_reference_count_increment(hsp_frame_metadata.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_frame_final)
{
    struct archi_context_hsp_frame_data *context_data =
        (struct archi_context_hsp_frame_data*)context;

    archi_hsp_frame_t *hsp_frame = context_data->frame.ptr;

    archi_pointer_t state_array = {
        .ptr = hsp_frame->state,
        .element = {
            .num_of = hsp_frame->num_states,
            .size = sizeof(*hsp_frame->state),
            .alignment = alignof(archi_hsp_state_t),
        },
    };

    archi_reference_count_decrement(context_data->frame_metadata.ref_count);
    archi_resize_array(&state_array, &context_data->frame_state, 0, NULL);
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
            .ptr = &hsp_frame->num_states,
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

        *value = context_data->frame_state[slot.index[0]];
    }
    else if (strcmp("metadata", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->frame_metadata;
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

    if (strcmp("num_states", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        size_t new_num_states = *(size_t*)value.ptr;

        archi_pointer_t state_array = {
            .ptr = hsp_frame->state,
            .element = {
                .num_of = hsp_frame->num_states,
                .size = sizeof(*hsp_frame->state),
                .alignment = alignof(archi_hsp_state_t),
            },
        };

        archi_hsp_state_t null_state = {0};

        archi_status_t code = archi_resize_array(&state_array,
                &context_data->frame_state, new_num_states, &null_state);
        if (code != 0)
            return code;

        hsp_frame->state = state_array.ptr;
        hsp_frame->num_states = new_num_states;
    }
    else if (strcmp("state", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= hsp_frame->num_states))
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->frame_state[slot.index[0]].ref_count);

        if (value.ptr != NULL)
            hsp_frame->state[slot.index[0]] = *(archi_hsp_state_t*)value.ptr;
        else
            hsp_frame->state[slot.index[0]] = (archi_hsp_state_t){0};

        context_data->frame_state[slot.index[0]] = value;
    }
    else if (strcmp("metadata", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->frame_metadata.ref_count);

        hsp_frame->metadata = value.ptr;
        context_data->frame_metadata = value;
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

/*****************************************************************************/

struct archi_context_hsp_branch_state_data_data {
    archi_pointer_t state_data;

    archi_pointer_t branch_selector_fn;
    archi_pointer_t branch_selector_data;
    archi_pointer_t *branch_frame;
};

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

    struct archi_context_hsp_branch_state_data_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_hsp_branch_state_data_t *branch_state_data = malloc(sizeof(*branch_state_data));
    if (branch_state_data == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *branch_state_data = (archi_hsp_branch_state_data_t){
        .num_branches = num_branches,
        .selector_fn = (archi_hsp_branch_selector_func_t)selector_fn.fptr,
        .selector_data = selector_data.ptr,
    };

    *context_data = (struct archi_context_hsp_branch_state_data_data){
        .state_data = {
            .ptr = branch_state_data,
            .element = {
                .num_of = 1,
                .size = sizeof(*branch_state_data),
                .alignment = alignof(archi_hsp_branch_state_data_t),
            },
        },
        .branch_selector_fn = selector_fn,
        .branch_selector_data = selector_data,
    };

    {
        archi_pointer_t frame_array = {
            .ptr = branch_state_data->branch,
            .element = {
                .size = sizeof(*branch_state_data->branch),
                .alignment = alignof(archi_hsp_frame_t),
            },
        };

        archi_hsp_frame_t null_frame = {0};

        archi_status_t code = archi_resize_array(&frame_array,
                &context_data->branch_frame, num_branches, &null_frame);
        if (code != 0)
        {
            free(branch_state_data);
            free(context_data);
            return code;
        }

        branch_state_data->branch = frame_array.ptr;
    }

    archi_reference_count_increment(selector_fn.ref_count);
    archi_reference_count_increment(selector_data.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_branch_state_data_final)
{
    struct archi_context_hsp_branch_state_data_data *context_data =
        (struct archi_context_hsp_branch_state_data_data*)context;

    archi_hsp_branch_state_data_t *branch_state_data = context_data->state_data.ptr;

    archi_pointer_t frame_array = {
        .ptr = branch_state_data->branch,
        .element = {
            .num_of = branch_state_data->num_branches,
            .size = sizeof(*branch_state_data->branch),
            .alignment = alignof(archi_hsp_frame_t),
        },
    };

    archi_reference_count_decrement(context_data->branch_selector_fn.ref_count);
    archi_reference_count_decrement(context_data->branch_selector_data.ref_count);
    archi_resize_array(&frame_array, &context_data->branch_frame, 0, NULL);
    free(branch_state_data);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_branch_state_data_get)
{
    struct archi_context_hsp_branch_state_data_data *context_data =
        (struct archi_context_hsp_branch_state_data_data*)context;

    archi_hsp_branch_state_data_t *branch_state_data = context_data->state_data.ptr;

    if (strcmp("num_branches", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &branch_state_data->num_branches,
            .ref_count = context_data->state_data.ref_count,
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
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= branch_state_data->num_branches))
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->branch_frame[slot.index[0]];
    }
    else if (strcmp("selector_fn", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->branch_selector_fn;
    }
    else if (strcmp("selector_data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->branch_selector_data;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_branch_state_data_set)
{
    struct archi_context_hsp_branch_state_data_data *context_data =
        (struct archi_context_hsp_branch_state_data_data*)context;

    archi_hsp_branch_state_data_t *branch_state_data = context_data->state_data.ptr;

    if (strcmp("num_branches", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        size_t new_num_branches = *(size_t*)value.ptr;

        archi_pointer_t frame_array = {
            .ptr = branch_state_data->branch,
            .element = {
                .num_of = branch_state_data->num_branches,
                .size = sizeof(*branch_state_data->branch),
                .alignment = alignof(archi_hsp_frame_t),
            },
        };

        archi_hsp_frame_t null_frame = {0};

        archi_status_t code = archi_resize_array(&frame_array,
                &context_data->branch_frame, new_num_branches, &null_frame);
        if (code != 0)
            return code;

        branch_state_data->branch = frame_array.ptr;
        branch_state_data->num_branches = new_num_branches;
    }
    else if (strcmp("branch", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= branch_state_data->num_branches))
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->branch_frame[slot.index[0]].ref_count);

        if (value.ptr != NULL)
            branch_state_data->branch[slot.index[0]] = *(archi_hsp_frame_t*)value.ptr;
        else
            branch_state_data->branch[slot.index[0]] = (archi_hsp_frame_t){0};

        context_data->branch_frame[slot.index[0]] = value;
    }
    else if (strcmp("selector_fn", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->branch_selector_fn.ref_count);

        branch_state_data->selector_fn = (archi_hsp_branch_selector_func_t)value.fptr;
        context_data->branch_selector_fn = value;
    }
    else if (strcmp("selector_data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->branch_selector_data.ref_count);

        branch_state_data->selector_data = value.ptr;
        context_data->branch_selector_data = value;
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

