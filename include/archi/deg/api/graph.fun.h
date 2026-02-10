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
#ifndef _ARCHI_DEG_API_GRAPH_FUN_H_
#define _ARCHI_DEG_API_GRAPH_FUN_H_

#include "archi/deg/api/node.typ.h"

/**
 * @brief Execute a directed graph.
 *
 * The algorithm is as following:
 * 0. set entry node as the current;
 * 1. call all non-null functions in the current node sequence,
 *    terminating with the error if any function returns non-zero error code;
 * 2. if the current branch selector function is non-null,
 *    call it to get a branch index and proceed to step 5;
 * 3. if the current branch selector function data is non-null,
 *    interpret it as a branch index and proceed to step 5;
 * 4. otherwise, set the index to 0;
 * 5. if the index is the special termination value, terminate;
 * 6. if the index is 0, set the default branch as the current node
 *    and proceed to step 1;
 * 7. if the index is valid, set the corresponding alternative branch as the current node
 *    and proceed to step 1;
 * 8. otherwise terminate.
 */
void
archi_deg_execute(
        const archi_deg_node_t *node, ///< [in] Entry node.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

#endif // _ARCHI_DEG_API_GRAPH_FUN_H_

