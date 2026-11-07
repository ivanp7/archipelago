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

#include "archi/context/i/interface.typ.h"


/**
 * @brief Context interface: directed execution graph node.
 *
 * Initialization parameters:
 * - "name"             : (char[]) node name
 * - "metadata"         : node metadata
 * - "sequence_length"  : (size_t) number of functions in the node sequence
 * - "transition_func"  : (archi_dexgraph_transition_func_t) transition function
 * - "transition_data"  : data of transition function, or the target node
 *
 * Getter slots:
 * - "name"                         : (char[]) node name
 * - "metadata"                     : node metadata
 * - "sequence.length"              : (size_t) number of functions in the node sequence
 * - "sequence.function" [index]    : (archi_dexgraph_operation_func_t) operation function #index
 * - "sequence.data" [index]        : data of operation function #index
 * - "transition.function"          : (archi_dexgraph_transition_func_t) transition function
 * - "transition.data"              : data of transition function, or the target node
 *
 * Calls:
 * - "execute"  : execute graph with this node as entry node
 *      returns: <nothing>
 *      parameters: <none>
 *
 * Setter slots:
 * - "name"                         : (char[]) node name
 * - "metadata"                     : node metadata
 * - "sequence.function" [index]    : (archi_dexgraph_operation_func_t) operation function #index
 * - "sequence.data" [index]        : data of operation function #index
 * - "transition.function"          : (archi_dexgraph_transition_func_t) transition function
 * - "transition.data"              : data of transition function, or the target node
 */
extern
const archi_context_interface_t
archi_context_interface__dexgraph_node;

#endif // _ARCHI_EXEC_CTX_NODE_VAR_H_

