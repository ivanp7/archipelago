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
 * @brief Functions for plugin state implementation.
 */

#pragma once
#ifndef _ARCHI_FSM_STATE_FUN_H_
#define _ARCHI_FSM_STATE_FUN_H_

#include "archi/fsm/state.typ.h"
#include "archi/fsm/state.def.h"
#include "archi/util/status.typ.h"

/**
 * @brief Get current state.
 *
 * If context is NULL, the function returns null state.
 *
 * @see ARCHI_CURRENT
 *
 * @return Current state.
 */
archi_state_t
archi_current(
        const struct archi_state_context *context ///< [in] State context.
);

/*****************************************************************************/

/**
 * @brief Exit the application immediately.
 *
 * Remaining states on the stack are discarded.
 *
 * If context is NULL, the function does nothing.
 *
 * @see ARCHI_EXIT
 */
void
archi_exit(
        archi_status_t exit_code, ///< [in] Exit code.

        struct archi_state_context *context ///< [in] State context.
);

/**
 * @brief Make the current state final and stop its execution.
 *
 * If context is NULL, the function does nothing.
 *
 * @see ARCHI_FINISH
 */
void
archi_finish(
        struct archi_state_context *context ///< [in] State context.
);

/**
 * @brief Proceed to the next state.
 *
 * archi_proceed(ARCHI_NULL_STATE) is equivalent to archi_finish().
 *
 * If context is NULL, the function does nothing.
 *
 * @see ARCHI_PROCEED
 */
void
archi_proceed(
        archi_state_t next_state, ///< [in] Next state.

        struct archi_state_context *context ///< [in] State context.
);

/**
 * @brief Call the next state.
 *
 * archi_call(ARCHI_NULL_STATE, ARCHI_NULL_STATE) is equivalent to archi_finish().
 * archi_call(state, ARCHI_NULL_STATE) is equivalent to archi_proceed(state).
 *
 * If the next state is null, it is popped from the stack.
 * If the return state is not null, it is pushed to the stack.
 *
 * If context is NULL, the function does nothing.
 *
 * @see ARCHI_CALL
 */
void
archi_call(
        archi_state_t next_state,   ///< [in] State to call.
        archi_state_t return_state, ///< [in] State to proceed to after the call has finished.

        struct archi_state_context *context ///< [in] State context.
);

/*****************************************************************************/

/**
 * @brief State function that executes a chain of states.
 */
ARCHI_STATE_FUNCTION(archi_state_chain_execute);

#endif // _ARCHI_FSM_STATE_FUN_H_

