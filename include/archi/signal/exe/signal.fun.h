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
 * @brief Transition functions for signaled conditions.
 */

#pragma once
#ifndef _ARCHI_SIGNAL_EXE_SIGNAL_FUN_H_
#define _ARCHI_SIGNAL_EXE_SIGNAL_FUN_H_

#include "archi/exec/api/transition.typ.h"


struct archi_dexgraph_transition_data__signal_detect;

/**
 * @brief Allocate a archi_dexgraph_transition_data__signal_detect_t object.
 *
 * @note The returned pointer must eventually be released via free().
 *
 * @return Pointer to a new archi_dexgraph_transition_data__signal_detect_t object, or NULL in case of failure.
 */
struct archi_dexgraph_transition_data__signal_detect*
archi_dexgraph_transition_data__signal_detect_alloc(
        size_t num_signals ///< Number of signal-branch associations.
);

/**
 * @brief Transition function: detect any of the specified signals.
 *
 * Function data type: archi_dexgraph_transition_data__signal_detect_t.
 *
 * @return 0 if none of the signal flags is set, specified branch index otherwise.
 */
ARCHI_DEXGRAPH_TRANSITION_FUNC(archi_dexgraph_transition__signal_detect);

#endif // _ARCHI_SIGNAL_EXE_SIGNAL_FUN_H_

