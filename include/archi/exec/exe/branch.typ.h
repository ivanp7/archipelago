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
 * @brief Data for transition functions for branching in directed execution graphs.
 */

#pragma once
#ifndef _ARCHI_EXEC_EXE_BRANCH_TYP_H_
#define _ARCHI_EXEC_EXE_BRANCH_TYP_H_

#include "archi/exec/exe/condition.typ.h"

#include <stdbool.h>


struct archi_dexgraph_node;

/**
 * @brief Transition function data: branch.
 */
typedef struct archi_dexgraph_transition_data__branch {
    archi_dexgraph_condition_func_t function; ///< Condition function.
    archi_pointer_t data; ///< Condition function data.

    const struct archi_dexgraph_node *target[2]; ///< Branch targets for both condition cases.
} archi_dexgraph_transition_data__branch_t;

#endif // _ARCHI_EXEC_EXE_BRANCH_TYP_H_

