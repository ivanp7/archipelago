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
 * @brief Constants and macros for implementation of plugin states.
 */

#pragma once
#ifndef _ARCHI_FSM_STATE_DEF_H_
#define _ARCHI_FSM_STATE_DEF_H_

#include "archi/fsm/state.typ.h"

/**
 * @brief Null (empty, invalid) state.
 */
#define ARCHI_NULL_STATE (archi_state_t){0}

/**
 * @brief State.
 */
#define ARCHI_STATE(func, data_ptr) \
    (archi_state_t){.function = (func), .data = (data_ptr)}

/*****************************************************************************/

/**
 * @brief Access current state.
 */
#define ARCHI_CURRENT archi_current(context)

/**
 * @brief Access current state data through a pointer of desired type.
 */
#define ARCHI_STATE_DATA(type) ((type*)ARCHI_CURRENT.data)

/*****************************************************************************/

/**
 * @brief Exit the application immediately.
 */
#define ARCHI_EXIT(exit_code) do {    \
    archi_exit((exit_code), context); \
} while (0)

/**
 * @brief Make the current state final and stop its execution.
 */
#define ARCHI_FINISH() do { \
    archi_finish(context);  \
} while (0)

/**
 * @brief Redo the current state with new data.
 */
#define ARCHI_REDO(new_data) do { \
    archi_proceed(ARCHI_STATE(ARCHI_CURRENT.function, (new_data)), context); \
} while (0)

/**
 * @brief Proceed to the next state with the current data.
 */
#define ARCHI_PROCEED_SAME_DATA(new_func) do { \
    archi_proceed(ARCHI_STATE((new_func), ARCHI_CURRENT.data), context); \
} while (0)

/**
 * @brief Proceed to the next state.
 */
#define ARCHI_PROCEED(next_state) do {    \
    archi_proceed((next_state), context); \
} while (0)

/**
 * @brief Call the next state.
 */
#define ARCHI_CALL(call_state, return_state) do {         \
    archi_call((call_state), (return_state), context);    \
} while (0)

/*****************************************************************************/

/**
 * @brief Declarator of a state function.
 *
 * @warning Use of variable-length arrays is not allowed in state functions,
 * as it will lead to memory leaks due to the finite state machine implementation.
 *
 * @see archi_state_function_t
 */
#define ARCHI_STATE_FUNCTION(name) void name( \
        struct archi_state_context *const context)

#endif // _ARCHI_FSM_STATE_DEF_H_

