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
 * @brief Data of transition functions for signaled conditions.
 */

#pragma once
#ifndef _ARCHI_SIGNAL_EXE_SIGNAL_TYP_H_
#define _ARCHI_SIGNAL_EXE_SIGNAL_TYP_H_

#include <stddef.h> // for size_t


struct archi_signal_flags;
struct archi_dexgraph_node;

/**
 * @brief Association of a signal index to a corresponding target index.
 */
typedef struct archi_dexgraph_transition_data__signal_detect__signal {
    int number; ///< Signal number.
    const struct archi_dexgraph_node *target; ///< Target node for the signal.
} archi_dexgraph_transition_data__signal_detect__signal_t;

/**
 * @brief Transition function data: signal flags and array of 'signal index'-'target index' pairs.
 */
typedef struct archi_dexgraph_transition_data__signal_detect {
    const struct archi_signal_flags *flags; ///< Signal flags.

    const struct archi_dexgraph_node *default_target; ///< Target node for the absense of signals.

    const size_t num_signals; ///< Number of signal-to-target associations.
    archi_dexgraph_transition_data__signal_detect__signal_t signal[]; ///< Signal-to-target associations.
} archi_dexgraph_transition_data__signal_detect_t;

#endif // _ARCHI_SIGNAL_EXE_SIGNAL_TYP_H_

