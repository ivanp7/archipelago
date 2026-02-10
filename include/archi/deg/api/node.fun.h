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
#ifndef _ARCHI_DEG_API_NODE_FUN_H_
#define _ARCHI_DEG_API_NODE_FUN_H_

#include "archi/deg/api/node.typ.h"

/**
 * @brief Allocate a node.
 *
 * @return Newly allocated node.
 */
archi_deg_node_t*
archi_deg_node_alloc(
        size_t sequence_length, ///< [in] Length of the sequence of node functions.

        const archi_deg_node_t *def_branch, ///< [in] Default execution branch.
        size_t num_alt_branches, ///< [in] Number of alternative execution branches.

        archi_deg_branch_function_t branch_fn, ///< [in] Branch selector function.
        void *branch_data ///< Branch selector function data.
);

/**
 * @brief Deallocate a node.
 */
void
archi_deg_node_free(
        archi_deg_node_t *node ///< [in] Node.
);

#endif // _ARCHI_DEG_API_NODE_FUN_H_

