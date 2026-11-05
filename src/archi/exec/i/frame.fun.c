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
 * @brief Operations with directed execution graph frames.
 */

#include "archi/exec/i/frame.fun.h"
#include "archi/exec/i/node.fun.h"
#include "archi/exec/i/operation.fun.h"
#include "archi/exec/i/transition.fun.h"


size_t
archi_dexgraph_execute_node(
        archi_dexgraph_frame_t frame,
        ARCHI_ERROR_PARAM_DECL)
{
    if (frame.node == NULL)
    {
        ARCHI_ERROR_RESET();
        return ARCHI_DEXGRAPH_TRANS_INDEX;
    }

    archi_error_t error;
    ARCHI_ERROR_VAR_RESET(&error);

    for (; frame.op_index < frame.node->sequence_length; frame.op_index++)
    {
        archi_dexgraph_operation_call(frame.node->sequence[frame.op_index], &error);
        if (error.code != 0)
            goto break_loop;
    }
    frame.op_index = ARCHI_DEXGRAPH_TRANS_INDEX; // reached transition successfully

break_loop:
    ARCHI_ERROR_ASSIGN(error);
    return frame.op_index;
}

archi_dexgraph_frame_t
archi_dexgraph_execute(
        archi_dexgraph_frame_t frame,
        ARCHI_ERROR_PARAM_DECL)
{
    archi_error_t error;
    ARCHI_ERROR_VAR_RESET(&error);

    while (frame.node != NULL)
    {
        // Execute the sequence of operation functions
        frame.op_index = archi_dexgraph_execute_node(frame, &error);
        if (error.code != 0)
            break;

        // Get the transition target
        const archi_dexgraph_node_t *target =
            archi_dexgraph_transition_target(frame.node->transition, &error);
        if (error.code != 0)
            break;

        // Update the frame
        frame = (archi_dexgraph_frame_t){.node = target};
    }

    ARCHI_ERROR_ASSIGN(error);
    return frame;
}

