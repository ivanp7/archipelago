/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief The finite state machine algorithm.
 */

#pragma once
#ifndef _ARCHI_FSM_ALGORITHM_FUN_H_
#define _ARCHI_FSM_ALGORITHM_FUN_H_

#include "archi/fsm/state.typ.h"
#include "archi/fsm/transition.typ.h"
#include "archi/util/status.typ.h"

/**
 * @brief Run finite state machine.
 *
 * @return Status code.
 */
archi_status_t
archi_finite_state_machine(
        archi_state_t entry_state, ///< [in] Entry state.
        archi_state_transition_t state_transition ///< [in] Optional state transition.
);

#endif // _ARCHI_FSM_ALGORITHM_FUN_H_

