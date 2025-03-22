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

#include <stddef.h>

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
 * @brief State of a finite state machine.
 */
typedef struct archi_fsm_state {
    archi_fsm_state_function_t function; ///< State function.
    void *data; ///< State data.

    void *metadata; ///< State metadata (for transition functions).
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
 * @brief Declarator of a selector function.
 *
 * @return Index of the selected entity.
 */
#define ARCHI_FSM_SELECTOR_FUNC(name) size_t name( \
        void *const data) /* Data for the function to operate on. */

/**
 * @brief Selector function.
 */
typedef ARCHI_FSM_SELECTOR_FUNC((*archi_fsm_selector_func_t));

/*****************************************************************************/

/**
 * @brief Stack frame of a finite state machine.
 */
typedef struct archi_fsm_stack_frame {
    archi_fsm_state_t *states; ///< Sequence of states.
    size_t length;             ///< Number of states in the sequence.
} archi_fsm_stack_frame_t;

/**
 * @brief Data for a branch state.
 */
typedef struct archi_fsm_state_branch_data {
    archi_fsm_selector_func_t selector_fn; ///< Branch selector function.
    void *selector_data;                   ///< Branch selector data.

    archi_fsm_stack_frame_t *frames; ///< Array of branches.
} archi_fsm_state_branch_data_t;

#endif // _ARCHI_FSM_STATE_TYP_H_

