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

#pragma once
#ifndef _ARCHI_EXEC_I_NODE_FUN_H_
#define _ARCHI_EXEC_I_NODE_FUN_H_

#include "archi_base/error.typ.h"


struct archi_dexgraph_node;

/**
 * @brief Execute node operations and transition.
 *
 * The function executes the operation sequence of a node,
 * halting if any function returns non-zero error code.
 *
 * Then, the transition is used to determine the successor node.
 *
 * @return Pointer to the successor node.
 */
const struct archi_dexgraph_node*
archi_dexgraph_execute_node(
        const struct archi_dexgraph_node *node, ///< [in] Node.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Execute a directed graph starting at the specified entry node.
 *
 * This function implements the most basic graph execution algorithm:
 * 1. execute the current node operation functions;
 * 2. execute the current node transition function;
 * 3. update the current node;
 * 4. repeat until the current node is NULL.
 */
void
archi_dexgraph_execute(
        const struct archi_dexgraph_node *node, ///< [in] Entry node.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_EXEC_I_NODE_FUN_H_

