/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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

#include "archi/fsm/instance.typ.h"
#include "archi/util/status.typ.h"

/**
 * @brief Execute a finite state machine.
 *
 * The algorithm is as following:
 * 0. Push the entry state to the stack.
 * 1. Call the transition function if it's set.
 * 2. If the transition function is set and provided a transitional state,
 *      use it as the next state and go to step 5.
 * 3. If the stack is empty, exit.
 * 4. Pop the next state from the stack.
 * 5. Call the state function [it can pop or push multiple states from or to the stack].
 * 6. Go to step 1.
 *
 * @return Status code.
 */
archi_status_t
archi_fsm_execute(
        archi_fsm_t fsm ///< [in] Finite state machine.
);

#endif // _ARCHI_FSM_ALGORITHM_FUN_H_

