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
 * @brief Functions for directed execution graph nodes.
 */

#pragma once
#ifndef _ARCHI_EXEC_API_NODE_FUN_H_
#define _ARCHI_EXEC_API_NODE_FUN_H_

#include "archi/exec/i/node.typ.h"


/**
 * @brief Allocate a node.
 *
 * @note Node name is copied.
 *
 * @return Newly allocated node.
 */
archi_dexgraph_node_t*
archi_dexgraph_node_alloc(
        size_t sequence_length ///< [in] Length of the sequence of operation functions.
);

/**
 * @brief Deallocate a node.
 *
 * @note Node name is freed.
 */
void
archi_dexgraph_node_free(
        archi_dexgraph_node_t *node ///< [in] Node.
);

/**
 * @brief Get node operation with the specified index.
 *
 * The function checks for the index range.
 * The empty operation is returned for any invalid inputs.
 *
 * @return An operation with the specified index.
 */
archi_dexgraph_operation_t
archi_dexgraph_node_operation(
        const archi_dexgraph_node_t *node, ///< [in] Node.
        size_t op_index ///< [in] Index of an operation.
);

/**
 * @brief Get node branch with the specified index.
 *
 * The function checks for NULL pointers and the index range.
 * NULL is returned for any invalid inputs.
 *
 * @return Pointer to a node branch, or NULL if there is no such branch.
 */
const archi_dexgraph_node_t*
archi_dexgraph_node_branch(
        const archi_dexgraph_node_t *node, ///< [in] Node.
        size_t branch_index ///< [in] Index of a branch.
);

/*****************************************************************************/

/**
 * @brief Allocate a node array.
 *
 * @return Newly allocated node array.
 */
archi_dexgraph_node_array_t*
archi_dexgraph_node_array_alloc(
        size_t num_nodes ///< [in] Size of array of nodes.
);

/**
 * @brief Deallocate a node array.
 */
void
archi_dexgraph_node_array_free(
        archi_dexgraph_node_array_t *node_array ///< [in] Node array.
);

#endif // _ARCHI_EXEC_API_NODE_FUN_H_

