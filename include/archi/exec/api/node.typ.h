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
 * @brief Types for directed execution graph nodes.
 */

#pragma once
#ifndef _ARCHI_EXEC_API_NODE_TYP_H_
#define _ARCHI_EXEC_API_NODE_TYP_H_

#include "archi/exec/api/operation.typ.h"
#include "archi/exec/api/transition.typ.h"

#include <stddef.h> // for size_t


struct archi_dexgraph_node_array;

/**
 * @brief Directed execution graph node.
 */
typedef struct archi_dexgraph_node {
    char *name; ///< Node name.

    archi_dexgraph_transition_t transition; ///< Execution transition.
    const struct archi_dexgraph_node_array *branch; ///< Array of branches to transit execution to.

    const size_t sequence_length; ///< Length of the node sequence.
    archi_dexgraph_operation_t sequence[]; ///< Node sequence.
} archi_dexgraph_node_t;

/**
 * @brief Array of directed execution graph nodes.
 */
typedef struct archi_dexgraph_node_array {
    const size_t num_nodes; ///< Number of nodes.
    const archi_dexgraph_node_t *node[]; ///< Array of nodes.
} archi_dexgraph_node_array_t;

#endif // _ARCHI_EXEC_API_NODE_TYP_H_

