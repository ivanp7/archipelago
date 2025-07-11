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
 * @brief The hierarchical state processor execution operation.
 */

#pragma once
#ifndef _ARCHI_HSP_EXEC_FUN_H_
#define _ARCHI_HSP_EXEC_FUN_H_

#include "archi/hsp/state.typ.h"
#include "archi/hsp/transition.typ.h"
#include "archi/util/status.typ.h"

/**
 * @brief Execute a hierarchical state processor.
 *
 * The algorithm is as following:
 * 0. Push the entry state to the stack.
 * 1. Call the transition function if it's provided, otherwise skip step 2.
 * 2. If the transition function returned a transitional state, use it as the next state and go to step 5.
 * 3. If the stack is empty, exit.
 * 4. Pop the next state from the stack.
 * 5. Call the state function [it can modify the stack].
 * 6. Go to step 1.
 *
 * @return Status code.
 */
archi_status_t
archi_hsp_execute(
        archi_hsp_state_t entry_state,    ///< [in] Entry (initial) state.
        archi_hsp_transition_t transition ///< [in] State transition.
);

#endif // _ARCHI_HSP_EXEC_FUN_H_

