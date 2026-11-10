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
 * @brief Operations with directed execution graph nodes.
 */

#include "archi/exec/i/node.fun.h"
#include "archi/exec/i/node.typ.h"
#include "archi/exec/i/operation.fun.h"
#include "archi/exec/i/transition.fun.h"


const archi_dexgraph_node_t*
archi_dexgraph_execute_node(
        const archi_dexgraph_node_t *node,
        ARCHI_ERROR_PARAM_DECL)
{
    if (node == NULL)
    {
        ARCHI_ERROR_RESET();
        return NULL;
    }

    archi_error_t error;
    const archi_dexgraph_node_t *target = NULL;

    // Execute the sequence of operation functions
    archi_dexgraph_operation_seq_exec(node->sequence, 0, &error);

    if (error.code != 0)
        goto finish;

    // Execute the transition function
    node = archi_dexgraph_transition_target(node->transition, &error);

    if (error.code != 0)
        goto finish;

    // Update the node
    target = node;

finish:
    ARCHI_ERROR_ASSIGN(error);
    return target;
}

void
archi_dexgraph_execute(
        const archi_dexgraph_node_t *node,
        ARCHI_ERROR_PARAM_DECL)
{
    archi_error_t error;
    ARCHI_ERROR_VAR_RESET(&error);

    while ((node != NULL) && (error.code == 0))
        node = archi_dexgraph_execute_node(node, &error);

    ARCHI_ERROR_ASSIGN(error);
}

