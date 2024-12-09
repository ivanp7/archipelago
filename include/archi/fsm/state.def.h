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
 * @brief Constants and macros for implementation of finite state machine states.
 */

#pragma once
#ifndef _ARCHI_FSM_STATE_DEF_H_
#define _ARCHI_FSM_STATE_DEF_H_

#include "archi/fsm/state.typ.h"

/**
 * @brief A null (empty) state.
 */
#define ARCHI_NULL_STATE (archi_state_t){0}

/**
 * @brief State literal.
 */
#define ARCHI_STATE(func, data_ptr) \
    (archi_state_t){.function = (func), .data = (data_ptr)}

/**
 * @brief State literal with function from another state.
 */
#define ARCHI_STATE_OTHER_DATA(state, data) ARCHI_STATE((state).function, (data))
/**
 * @brief State literal with data from another state.
 */
#define ARCHI_STATE_OTHER_FUNC(state, func) ARCHI_STATE((func), (state).data)

/*****************************************************************************/

/**
 * @brief Access current state.
 */
#define ARCHI_CURRENT() archi_current(fsm)
/**
 * @brief Access current state data through a pointer to the specified type.
 */
#define ARCHI_STATE_DATA(type) ((type*)ARCHI_CURRENT().data)

/**
 * @brief Access current stack size.
 */
#define ARCHI_STACK_SIZE() archi_stack_size(fsm)

/**
 * @brief Access current status code.
 */
#define ARCHI_CODE() archi_code(fsm)
/**
 * @brief Update status code.
 */
#define ARCHI_SET_CODE(code) archi_set_code(fsm, (code))

/*****************************************************************************/

/**
 * @brief Proceed finite state machine execution -- pop and/or push states from/to the stack.
 */
#define ARCHI_PROCEED(num_popped, ...) do { \
    archi_state_t seq[] = {__VA_ARGS__};    \
    archi_proceed(fsm, (num_popped), sizeof(seq) / sizeof(seq[0]), seq); \
} while (0)

/**
 * @brief Proceed finite state machine execution -- pop states from the stack.
 */
#define ARCHI_DONE(num_popped) do { \
    archi_proceed(fsm, (num_popped), 0, NULL); \
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
        struct archi_finite_state_machine_context *const fsm)

#endif // _ARCHI_FSM_STATE_DEF_H_

