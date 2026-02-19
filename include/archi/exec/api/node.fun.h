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

#include "archi/exec/api/node.typ.h"


/**
 * @brief Allocate a node.
 *
 * @note Node name is copied.
 *
 * @return Newly allocated node.
 */
archi_dexgraph_node_t*
archi_dexgraph_node_alloc(
        const char *name, ///< [in] Node name.
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

