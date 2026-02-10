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
#ifndef _ARCHI_DEG_API_NODE_TYP_H_
#define _ARCHI_DEG_API_NODE_TYP_H_

#include "archipelago/base/error.typ.h"

#include <stddef.h> // for size_t

/**
 * @brief Signature of a node function.
 */
#define ARCHI_DEG_NODE_FUNCTION(name)   void name(  \
        void *data, /* [in] Node function data. */  \
        ARCHI_ERROR_PARAMETER_DECL) /* [out] Error. */

/**
 * @brief Node function type.
 */
typedef ARCHI_DEG_NODE_FUNCTION((*archi_deg_node_function_t));

/**
 * @brief Function type tag for node functions.
 */
#define ARCHI_POINTER_FUNCTION_TAG__DEG_NODE    2

/**
 * @brief Directed execution graph node function with data.
 */
typedef struct archi_deg_node_func_with_data {
    archi_deg_node_function_t function; ///< Node function.
    void *data; ///< Node function data.
} archi_deg_node_func_with_data_t;

/*****************************************************************************/

/**
 * @brief Signature of a branch selector function.
 *
 * Index 0 corresponds to the default branch,
 * higher indices correspond to alternative branches
 * (array index is shifted by one).
 *
 * @return Index of the selected branch, or the execution terminator value.
 */
#define ARCHI_DEG_BRANCH_FUNCTION(name)   size_t name( \
        void *data) /* [in] Node branch function data. */

/**
 * @brief Node branch selector function type.
 */
typedef ARCHI_DEG_BRANCH_FUNCTION((*archi_deg_branch_function_t));

/**
 * @brief Function type tag for branch selector functions.
 */
#define ARCHI_POINTER_FUNCTION_TAG__DEG_BRANCH  3

/**
 * @brief Directed execution graph terminator value.
 */
#define ARCHI_DEG_TERMINATE     ((size_t)-1)

/*****************************************************************************/

/**
 * @brief Directed execution graph node.
 */
typedef struct archi_deg_node {
    const struct archi_deg_node *def_branch; ///< Default execution branch.

    size_t num_alt_branches; ///< Number of alternative execution branches.
    const struct archi_deg_node **alt_branch; ///< Array of alternative execution branches.

    struct {
        archi_deg_branch_function_t function; ///< Branch selector function.
        void *data; ///< Branch selector function data.
    } branch_selector;

    const size_t sequence_length; ///< Length of the sequence of node functions.
    archi_deg_node_func_with_data_t sequence[]; ///< Sequence of node functions.
} archi_deg_node_t;

#endif // _ARCHI_DEG_API_NODE_TYP_H_

