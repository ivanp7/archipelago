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
 * @brief Types for finite state machine states.
 */

#pragma once
#ifndef _ARCHI_FSM_STATE_TYP_H_
#define _ARCHI_FSM_STATE_TYP_H_

struct archi_fsm_context;

/**
 * @brief Declarator of a state function.
 *
 * @warning Use of variable-length arrays is not allowed in state functions,
 * as it will lead to memory leaks due to the finite state machine implementation.
 */
#define ARCHI_FSM_STATE_FUNCTION(name) void name( \
        struct archi_fsm_context *const fsm) /* Finite state machine context. */

/**
 * @brief State function.
 */
typedef ARCHI_FSM_STATE_FUNCTION((*archi_fsm_state_function_t));

/*****************************************************************************/

/**
 * @brief State.
 */
typedef struct archi_fsm_state {
    archi_fsm_state_function_t function; ///< State function.
    void *data; ///< State data.

    void *metadata; ///< State metadata (for debugging purposes).
} archi_fsm_state_t;

/**
 * @brief Null (empty) state.
 */
#define ARCHI_NULL_FSM_STATE (archi_fsm_state_t){0}

/**
 * @brief State literal.
 */
#define ARCHI_FSM_STATE(func, data_ptr) \
    (archi_fsm_state_t){.function = (func), .data = (data_ptr)}
/**
 * @brief State literal (with metadata).
 */
#define ARCHI_FSM_STATE_M(func, data_ptr, metadata_ptr) \
    (archi_fsm_state_t){.function = (func), .data = (data_ptr), .metadata = (metadata_ptr)}

/**
 * @brief State literal with function from another state.
 */
#define ARCHI_FSM_STATE_OTHER_DATA(state, data) ARCHI_FSM_STATE_M((state).function, (data), (state).metadata)
/**
 * @brief State literal with data from another state.
 */
#define ARCHI_FSM_STATE_OTHER_FUNC(state, func) ARCHI_FSM_STATE_M((func), (state).data, (state).metadata)

/*****************************************************************************/

/**
 * @brief Chain (linked list) of states.
 */
typedef struct archi_fsm_state_chain {
    void *data; ///< Current state data (or pointer to the next node).
    archi_fsm_state_t next_state; ///< Next state.
} archi_fsm_state_chain_t;

#endif // _ARCHI_FSM_STATE_TYP_H_

