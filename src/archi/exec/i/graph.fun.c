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
 * @brief Directed execution graph operations.
 */

#include "archi/exec/i/graph.fun.h"
#include "archi/exec/i/node.fun.h"


void
archi_dexgraph_execute_node_op(
        archi_dexgraph_frame_t frame,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((frame.node == NULL) || (frame.op_index >= frame.node->sequence_length))
    {
        ARCHI_ERROR_RESET();
        return;
    }

    archi_dexgraph_operation_t operation = frame.node->sequence[frame.op_index];

    if (operation.function != NULL)
    {
        ARCHI_ERROR_UNSET();
        /****************************************************/
        operation.function(operation.data, ARCHI_ERROR_PARAM);
        /****************************************************/
    }
    else
        ARCHI_ERROR_RESET();
}

size_t
archi_dexgraph_execute_node_ops(
        archi_dexgraph_frame_t frame,
        ARCHI_ERROR_PARAM_DECL)
{
    if (frame.node == NULL)
    {
        ARCHI_ERROR_RESET();
        return frame.op_index;
    }

    archi_error_t error;

    for (; frame.op_index < frame.node->sequence_length; frame.op_index++)
    {
        archi_dexgraph_execute_node_op(frame, &error);
        if (error.code != 0)
            break;
    }

    ARCHI_ERROR_ASSIGN(error);
    return frame.op_index;
}

size_t
archi_dexgraph_execute_node_trans(
        const archi_dexgraph_node_t *node,
        ARCHI_ERROR_PARAM_DECL)
{
    if (node == NULL)
    {
        ARCHI_ERROR_RESET();
        return ARCHI_DEXGRAPH_HALT;
    }

    archi_dexgraph_transition_t transition = node->transition;

    if (transition.function != NULL)
    {
        ARCHI_ERROR_UNSET();
        /*************************************************************/
        return transition.function(transition.data, ARCHI_ERROR_PARAM);
        /*************************************************************/
    }
    else
    {
        ARCHI_ERROR_RESET();
        return (transition.data != NULL) ? *(size_t*)transition.data : 0;
    }
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
        frame.op_index = archi_dexgraph_execute_node_ops(frame, &error);
        if (error.code != 0)
            break;

        // Execute the transition function
        size_t branch_index = archi_dexgraph_execute_node_trans(frame.node, &error);
        if (error.code != 0)
            break;

        // Obtain the selected branch
        frame.node = archi_dexgraph_node_branch(frame.node, branch_index);
        frame.op_index = 0;
    }

    ARCHI_ERROR_ASSIGN(error);
    return frame;
}

