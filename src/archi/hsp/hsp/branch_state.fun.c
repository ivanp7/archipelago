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
 * @brief Hierarchical state processor states for execution branching.
 */

#include "archi/hsp/hsp/branch_state.fun.h"
#include "archi/hsp/api/state.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h> // for malloc(), rand()

archi_hsp_state_data__branch_t*
archi_hsp_state_data_alloc__branch(
        size_t num_branches,
        archi_hsp_branch_selector_func_t selector_fn,
        void *selector_data)
{
    archi_hsp_state_data__branch_t *state_data = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archi_hsp_state_data__branch_t, branch, num_branches));
    if (state_data == NULL)
        return NULL;

    size_t *num_branches_ptr = (size_t*)&state_data->num_branches;
    *num_branches_ptr = num_branches;

    state_data->selector_fn = selector_fn;
    state_data->selector_data = selector_data;

    for (size_t i = 0; i < num_branches; i++)
        state_data->branch[i] = NULL;

    return state_data;
}

/*****************************************************************************/

ARCHI_HSP_STATE_FUNCTION(archi_hsp_state__advance)
{
    (void) ARCHI_ERROR_PARAMETER;

    const archi_hsp_frame_t *frame = data;
    if (frame == NULL)
        return;

    archi_hsp_advance(hsp, 0, frame->num_states, frame->state);
}

ARCHI_HSP_STATE_FUNCTION(archi_hsp_state__branch)
{
    const archi_hsp_state_data__branch_t *state_data = data;
    if ((state_data == NULL) || (state_data->num_branches == 0))
        return;

    size_t index = 0;
    if (state_data->selector_fn != NULL)
        index = state_data->selector_fn(state_data->num_branches, state_data->selector_data);

    if (index >= state_data->num_branches)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "branch index (#%zu) out of range (%zu branches)",
                index, state_data->num_branches);
        return;
    }

    const archi_hsp_frame_t *frame = state_data->branch[index];
    archi_hsp_advance(hsp, 0, frame->num_states, frame->state);
}

/*****************************************************************************/

ARCHI_HSP_BRANCH_SELECTOR_FUNC(archi_hsp_branch_select__uncond)
{
    (void) num_branches;
    return (data != NULL) ? *(size_t*)data : 0;
}

ARCHI_HSP_BRANCH_SELECTOR_FUNC(archi_hsp_branch_select__random)
{
    (void) data;
    return rand() % num_branches;
}

ARCHI_HSP_BRANCH_SELECTOR_FUNC(archi_hsp_branch_select__loop)
{
    (void) num_branches;

    size_t *loop_control = data;
    if (loop_control == NULL)
        return 1;

    if (loop_control[1] < loop_control[0])
    {
        loop_control[1]++;
        return 0;
    }
    else
    {
        loop_control[1] = 0;
        return 1;
    }
}

