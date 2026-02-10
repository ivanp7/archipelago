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
#ifndef _ARCHI_DEG_CTX_NODE_VAR_H_
#define _ARCHI_DEG_CTX_NODE_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: directed execution graph node.
 *
 * Initialization parameters:
 * - "sequence_length"  : (size_t) number of functions in the node
 * - "default_branch"   : (archi_deg_node_t) default branch
 * - "alt_branches"     : (archi_deg_node_t[]) array of alternative branches
 * - "branch_function"  : (archi_deg_branch_function_t) branch selector function
 * - "branch_func_data" : branch selector function data
 *
 * Getter slots:
 * - "sequence.length"              : (size_t) number of functions in the node
 * - "sequence.function" [index]    : (archi_deg_node_function_t) node function #index
 * - "sequence.func_data" [index]   : node function data #index
 * - "branch.default"               : (archi_deg_node_t) default branch
 * - "branch.alternatives"          : (archi_deg_node_t*[]) array of alternative branches
 * - "branch.num_alternatives"      : (size_t) number of alternative branches
 * - "branch.function"              : (archi_deg_branch_function_t) branch selector function
 * - "branch.func_data"             : branch selector function data
 *
 * Calls:
 * - "execute"  : execute DEG with this node as entry node
 *      <no parameters>
 *
 * Setter slots:
 * - "sequence.function" [index]    : (archi_deg_node_function_t) node function #index
 * - "sequence.func_data" [index]   : node function data #index
 * - "branch.default"               : (archi_deg_node_t) default branch
 * - "branch.alternatives"          : (archi_deg_node_t*[]) array of alternative branches
 * - "branch.function"              : (archi_deg_branch_function_t) branch selector function
 * - "branch.func_data"             : branch selector function data
 */
extern
const archi_context_interface_t
archi_context_interface__deg_node;

#endif // _ARCHI_DEG_CTX_NODE_VAR_H_

