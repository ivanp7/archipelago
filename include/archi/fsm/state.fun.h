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
 * @brief Get number of frames on the FSM stack.
 *
 * If fsm is NULL, the function returns 0.
 *
 * @return Number of frames on the FSM stack.
 */
size_t
archi_fsm_stack_frames(
        const struct archi_fsm_context *fsm ///< [in] Finite state machine context.
);

/*****************************************************************************/

/**
 * @brief Access current state.
 */
#define ARCHI_FSM_CURRENT() archi_fsm_current(fsm)
/**
 * @brief Access current state data through a pointer to the specified type.
 */
#define ARCHI_FSM_CURRENT_DATA(type) ((type)ARCHI_FSM_CURRENT().data)
/**
 * @brief Access current state metadata through a pointer to the specified type.
 */
#define ARCHI_FSM_CURRENT_METADATA(type) ((type)ARCHI_FSM_CURRENT().metadata)

/**
 * @brief Access current number of frames on the stack.
 */
#define ARCHI_FSM_STACK_FRAMES() archi_fsm_stack_frames(fsm)

/*****************************************************************************/

/**
 * @brief Proceed finite state machine execution.
 *
 * This function pops the specified number of frames (sequences of states that are pushed at once),
 * then pushes a frame to the stack.
 * Null states in the frame are left out and not pushed.
 *
 * If the stack is empty after the pop operation and there are no states pushed,
 * the finite state machine exits.
 *
 * Returning from a state function normally is equivalent to calling
 * archi_proceed(fsm, 0, 0, NULL);
 *
 * If fsm is NULL, the function does nothing.
 * If the function is not called from a state function during finite state machine execution, it does nothing.
 * Otherwise, it does not return and triggers the state transition.
 */
void
archi_fsm_proceed(
        struct archi_fsm_context *fsm, ///< [in] Finite state machine context.

        size_t pop_frames, ///< [in] Number of frames to pop from the stack before pushing.

        const archi_fsm_state_t frame[], ///< [in] Frame to push to the stack.
        size_t frame_length              ///< [in] Number of states in the pushed frame.
);

/*****************************************************************************/

/**
 * @brief Proceed finite state machine execution.
 */
#define ARCHI_FSM_PROCEED(pop_frames, ...) do { \
    const archi_fsm_state_t _archi_frame[] = {__VA_ARGS__}; \
    archi_fsm_proceed(fsm, (pop_frames), _archi_frame, sizeof(_archi_frame) / sizeof(_archi_frame[0])); \
} while (0)

/**
 * @brief Proceed finite state machine execution without pushing a new frame.
 */
#define ARCHI_FSM_FINISH(pop_frames) do { \
    archi_fsm_proceed(fsm, (pop_frames), NULL, 0); \
} while (0)

/*****************************************************************************/

/**
 * @brief State function that executes a chain of states.
 */
ARCHI_FSM_STATE_FUNCTION(archi_fsm_state_chain_execute);

#endif // _ARCHI_FSM_STATE_FUN_H_

