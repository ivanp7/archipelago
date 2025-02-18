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
 * @brief Functions for implementation of finite state machine states.
 */

#pragma once
#ifndef _ARCHI_FSM_STATE_FUN_H_
#define _ARCHI_FSM_STATE_FUN_H_

#include "archi/fsm/state.typ.h"
#include "archi/util/status.typ.h"

#include <stddef.h>

/**
 * @brief Get current state.
 *
 * If fsm is NULL, the function returns null state.
 *
 * @return Current state.
 */
archi_fsm_state_t
archi_fsm_current(
        const struct archi_fsm_context *fsm ///< [in] Finite state machine context.
);

/**
 * @brief Get current stack size.
 *
 * If fsm is NULL, the function returns 0.
 *
 * @return Current stack size.
 */
size_t
archi_fsm_stack_size(
        const struct archi_fsm_context *fsm ///< [in] Finite state machine context.
);

/**
 * @brief Get current status code.
 *
 * If fsm is NULL, the function returns 0.
 *
 * @return Current status code.
 */
archi_status_t
archi_fsm_code(
        const struct archi_fsm_context *fsm ///< [in] Finite state machine context.
);

/**
 * @brief Set status code.
 *
 * If fsm is NULL, the function does nothing.
 * If called not from a state function during finite state machine execution,
 * the function does nothing.
 */
void
archi_fsm_set_code(
        struct archi_fsm_context *fsm, ///< [in] Finite state machine context.

        archi_status_t code ///< [in] Status code.
);

/*****************************************************************************/

/**
 * @brief Access current state.
 */
#define ARCHI_FSM_CURRENT() archi_fsm_current(fsm)
/**
 * @brief Access current state data through a pointer to the specified type.
 */
#define ARCHI_FSM_CURRENT_DATA(type) ((type*)ARCHI_FSM_CURRENT().data)
/**
 * @brief Access current state metadata through a pointer to the specified type.
 */
#define ARCHI_FSM_CURRENT_METADATA(type) ((type*)ARCHI_FSM_CURRENT().metadata)

/**
 * @brief Access current stack size.
 */
#define ARCHI_FSM_STACK_SIZE() archi_fsm_stack_size(fsm)

/**
 * @brief Access current status code.
 */
#define ARCHI_FSM_CODE() archi_fsm_code(fsm)
/**
 * @brief Update status code.
 */
#define ARCHI_FSM_SET_CODE(code) archi_fsm_set_code(fsm, (code))

/*****************************************************************************/

/**
 * @brief Proceed finite state machine execution -- pop and/or push states to the stack.
 *
 * This function pops the specified number of states and
 * pushes non-null states from the `pushed` array
 * to the stack in reverse order (high index to low index).
 * Null states are ignored.
 *
 * If the stack is empty after pop operation and no states are pushed,
 * the finite state machine exits with the specified status code.
 *
 * Returning from a state function normally is equivalent to:
 * archi_proceed(fsm, 0, 0, NULL);
 *
 * If fsm is NULL, the function does nothing.
 * If called not from a state function during finite state machine execution,
 * the function does nothing.
 */
void
archi_fsm_proceed(
        struct archi_fsm_context *fsm, ///< [in] Finite state machine context.

        size_t num_popped, ///< [in] Number of states popped from the stack (excluding the auto-pop to get the next state).
        size_t num_pushed, ///< [in] Number of states in the pushed states array.
        const archi_fsm_state_t pushed[] ///< [in] Array of states to be pushed to the stack.
);

/*****************************************************************************/

/**
 * @brief Proceed finite state machine execution -- pop and/or push states from/to the stack.
 */
#define ARCHI_FSM_PROCEED(num_popped, ...) do { \
    archi_fsm_state_t pushed[] = {__VA_ARGS__}; \
    archi_fsm_proceed(fsm, (num_popped), sizeof(pushed) / sizeof(pushed[0]), pushed); \
} while (0)

/**
 * @brief Proceed finite state machine execution -- pop states from the stack.
 */
#define ARCHI_FSM_DONE(num_popped) do { \
    archi_fsm_proceed(fsm, (num_popped), 0, NULL); \
} while (0)

/*****************************************************************************/

/**
 * @brief State function that executes a chain of states.
 */
ARCHI_FSM_STATE_FUNCTION(archi_fsm_state_chain_execute);

#endif // _ARCHI_FSM_STATE_FUN_H_

