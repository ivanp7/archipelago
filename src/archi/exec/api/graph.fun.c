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

#include "archi/exec/api/graph.fun.h"
#include "archi/exec/api/node.typ.h"
#include "archi/exec/api/transition.typ.h"
#include "archi_base/pointer.fun.h"


archi_dexgraph_frame_t
archi_dexgraph_execute(
        archi_dexgraph_frame_t frame,
        enum archi_dexgraph_exec_mode mode,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((mode < ARCHI_DEXGRAPH__NO_INTERRUPT) || (mode > ARCHI_DEXGRAPH__INTERRUPT_OPERATION))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unknown DEG execution mode %i", mode);
        return frame;
    }

    archi_error_t error;
    ARCHI_ERROR_VAR_RESET(&error);

    while (frame.node != NULL)
    {
        // Execute the sequence of operation functions
        for (; frame.index < frame.node->sequence_length; frame.index++)
        {
            archi_dexgraph_operation_t operation = frame.node->sequence[frame.index];

            if (operation.function != NULL)
            {
                ARCHI_ERROR_VAR_UNSET(&error);
                /*****************************************/
                operation.function(operation.data, &error);
                /*****************************************/

                if (error.code != 0)
                    goto interrupt;
                else if (mode >= ARCHI_DEXGRAPH__INTERRUPT_OPERATION)
                {
                    frame.index++;
                    goto interrupt;
                }
            }
        }

        // Select a branch to transit execution to
        archi_dexgraph_branch_index_t branch_index;
        {
            archi_dexgraph_transition_t transition = frame.node->transition;

            if (transition.function != NULL)
            {
                ARCHI_ERROR_VAR_UNSET(&error);
                /**********************************************************/
                branch_index = transition.function(transition.data, &error);
                /**********************************************************/

                if (error.code != 0)
                    goto interrupt;
            }
            else if (transition.data != NULL)
                branch_index = *(archi_dexgraph_branch_index_t*)transition.data;
            else
                branch_index = 0;
        }

        // Proceed to the selected branch
        const archi_dexgraph_node_array_t *branch = frame.node->branch;

        if ((branch != NULL) && (branch_index < branch->num_nodes))
            frame.node = branch->node[branch_index];
        else // non-existent branch or ARCHI_DEXGRAPH_HALT
            frame.node = NULL;

        frame.index = 0;

        if (mode >= ARCHI_DEXGRAPH__INTERRUPT_TRANSITION)
            goto interrupt;
    }
interrupt:

    ARCHI_ERROR_ASSIGN(error);
    return frame;
}

