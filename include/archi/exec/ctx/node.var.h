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
 * @brief Context interface for directed execution graph nodes.
 */

#pragma once
#ifndef _ARCHI_EXEC_CTX_NODE_VAR_H_
#define _ARCHI_EXEC_CTX_NODE_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: directed execution graph node.
 *
 * Initialization parameters:
 * - "name"             : (char[]) node name
 * - "sequence_length"  : (size_t) number of functions in the node sequence
 * - "transition_func"  : (archi_dexgraph_transition_func_t) transition function
 * - "transition_data"  : data of transition function
 * - "branches"         : (archi_dexgraph_node_array_t) array of branch nodes
 *
 * Getter slots:
 * - "name"                         : (char[]) node name
 * - "sequence.length"              : (size_t) number of functions in the node sequence
 * - "sequence.function" [index]    : (archi_dexgraph_operation_func_t) operation function #index
 * - "sequence.data" [index]        : data of operation function #index
 * - "transition.function"          : (archi_dexgraph_transition_func_t) transition function
 * - "transition.data"              : data of transition function
 * - "branches"                     : (archi_dexgraph_node_array_t) array of branch nodes
 *
 * Calls:
 * - "execute"  : execute DEG with this node as entry node
 *      returns: <nothing>
 *      parameters:
 *      - "index"   : (archi_dexgraph_branch_index_t) initial operation function index
 *
 * Setter slots:
 * - "sequence.function" [index]    : (archi_dexgraph_operation_func_t) operation function #index
 * - "sequence.data" [index]        : data of operation function #index
 * - "transition.function"          : (archi_dexgraph_transition_func_t) transition function
 * - "transition.data"              : data of transition function
 * - "branches"                     : (archi_dexgraph_node_array_t) array of branch nodes
 */
extern
const archi_context_interface_t
archi_context_interface__dexgraph_node;

/**
 * @brief Context interface: directed execution graph node array.
 *
 * Initialization parameters:
 * - "num_nodes"    : (size_t) number of nodes in array
 *
 * Getter slots:
 * - "num_nodes"    : (size_t) number of nodes in array
 * - "node" [index] : (archi_dexgraph_node_t) node #index
 *
 * Setter slots:
 * - "node" [index] : (archi_dexgraph_node_t) node #index
 */
extern
const archi_context_interface_t
archi_context_interface__dexgraph_node_array;

#endif // _ARCHI_EXEC_CTX_NODE_VAR_H_

