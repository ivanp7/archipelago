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
 * @brief Types for directed execution graph transitions.
 */

#pragma once
#ifndef _ARCHI_EXEC_API_TRANSITION_TYP_H_
#define _ARCHI_EXEC_API_TRANSITION_TYP_H_

#include "archi_base/error.typ.h"

#include <stddef.h> // for size_t


/**
 * @brief Index of a branch.
 */
typedef size_t archi_dexgraph_branch_index_t;

/**
 * @brief Special branch index value for DEG execution halting.
 */
#define ARCHI_DEXGRAPH_HALT  ((archi_dexgraph_branch_index_t)-1)

/**
 * @brief Signature of a transition function.
 *
 * @return Index of the selected branch, or ARCHI_DEXGRAPH_HALT.
 */
#define ARCHI_DEXGRAPH_TRANSITION_FUNC(func_name)   archi_dexgraph_branch_index_t func_name(    \
        void *data, /* [in] Transition function data. */                                        \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Transition function type.
 */
typedef ARCHI_DEXGRAPH_TRANSITION_FUNC((*archi_dexgraph_transition_func_t));

/**
 * @brief Directed execution graph transition.
 */
typedef struct archi_dexgraph_transition {
    archi_dexgraph_transition_func_t function; ///< Transition function.
    void *data; ///< Transition function data.
} archi_dexgraph_transition_t;

#endif // _ARCHI_EXEC_API_TRANSITION_TYP_H_

