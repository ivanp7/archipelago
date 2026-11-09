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
#include "archi/exec/i/node.typ.h"
#include "archi/exec/i/operation.fun.h"
#include "archi/exec/i/transition.fun.h"


archi_dexgraph_frame_t
archi_dexgraph_execute_node(
        archi_dexgraph_frame_t frame,
        ARCHI_ERROR_PARAM_DECL)
{
    if (frame.node == NULL)
    {
        ARCHI_ERROR_RESET();
        return (archi_dexgraph_frame_t){0};
    }

    archi_error_t error;

    // Execute the sequence of operation functions
    frame.op_index = archi_dexgraph_operation_seq_exec(frame.node->sequence, frame.op_index, &error);

    if (error.code != 0)
        goto finish;

    // Execute the transition function
    const archi_dexgraph_node_t *target =
        archi_dexgraph_transition_target(frame.node->transition, &error);

    if (error.code != 0)
        goto finish;

    // Update the frame
    frame = (archi_dexgraph_frame_t){.node = target};

finish:
    ARCHI_ERROR_ASSIGN(error);
    return frame;
}

archi_dexgraph_frame_t
archi_dexgraph_execute(
        archi_dexgraph_frame_t frame,
        ARCHI_ERROR_PARAM_DECL)
{
    archi_error_t error;
    ARCHI_ERROR_VAR_RESET(&error);

    while ((frame.node != NULL) && (error.code == 0))
        frame = archi_dexgraph_execute_node(frame, &error);

    ARCHI_ERROR_ASSIGN(error);
    return frame;
}

