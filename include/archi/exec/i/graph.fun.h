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

#pragma once
#ifndef _ARCHI_EXEC_API_GRAPH_FUN_H_
#define _ARCHI_EXEC_API_GRAPH_FUN_H_

#include "archi/exec/i/frame.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Execute a single operation at the specified frame.
 */
void
archi_dexgraph_execute_node_op(
        archi_dexgraph_frame_t frame, ///< [in] Frame.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Execute the rest of node operations starting at the specified frame.
 *
 * The function calls non-NULL operation functions in the current node sequence,
 * starting from the specified index, halting if any function returns non-zero error code.
 *
 * @return Index of the first operation not executed (successfully).
 */
size_t
archi_dexgraph_execute_node_ops(
        archi_dexgraph_frame_t frame, ///< [in] Frame.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Execute the node transition function.
 *
 * If the transition function is not NULL, it is called with the transition data
 * and its return value is the branch index.
 *
 * Otherwise, the transition data is interpreted as the branch index.
 * If the pointer is not NULL, it is dereferenced;
 * if it's NULL, the default value of `0` is used instead.
 *
 * @return Index of the chosen branch.
 */
size_t
archi_dexgraph_execute_node_trans(
        const struct archi_dexgraph_node *node, ///< [in] Node.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Execute a directed graph starting at the specified frame.
 *
 * This function implements the most basic graph execution algorithm:
 * 1. execute the current node operation functions;
 * 2. execute the current node transition function;
 * 3. translate the branch index using the current translation table (optional);
 * 4. update the current node;
 * 5. repeat until the current node is NULL.
 *
 * Branch index translation tables are stored in the `data` field of the node structure.
 *
 * A translation table is an array of `size_t`, where the first element
 * is the number of entries, and the rest are the mapping entries:
 * `new_index := table[old_index + 1]`, where `old_index < table[0]`.
 * Out-of-range indices are mapped to ARCHI_DEXGRAPH_HALT.
 * If a table is NULL, the branch index is mapped to itself.
 *
 * Returned frame contains NULL node if execution terminated without errors.
 * Otherwise, it stores the current node pointer and operation index
 * of the location where an error occured.
 *
 * @return Execution frame at the halt point.
 */
archi_dexgraph_frame_t
archi_dexgraph_execute(
        archi_dexgraph_frame_t frame, ///< [in] Frame.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_EXEC_API_GRAPH_FUN_H_

