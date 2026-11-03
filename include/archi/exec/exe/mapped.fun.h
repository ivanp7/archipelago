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
 * @brief Transition functions for branch index mapping in directed execution graphs.
 */

#pragma once
#ifndef _ARCHI_EXEC_EXE_MAPPED_FUN_H_
#define _ARCHI_EXEC_EXE_MAPPED_FUN_H_

#include "archi/exec/i/transition.typ.h"


/**
 * @brief Transition function: map branch index according to the table.
 *
 * Function data type: archi_dexgraph_transition_data__mapped_t.
 *
 * @return Mapped branch index, or ARCHI_DEXGRAPH_HALT by default.
 */
ARCHI_DEXGRAPH_TRANSITION_FUNC(archi_dexgraph_transition__mapped);

#endif // _ARCHI_EXEC_EXE_MAPPED_FUN_H_

