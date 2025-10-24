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
 * @brief Functions for implementation of hierarchical state processor states.
 */

#pragma once
#ifndef _ARCHI_HSP_API_STATE_FUN_H_
#define _ARCHI_HSP_API_STATE_FUN_H_

#include "archi/hsp/api/state.typ.h"
#include "archipelago/base/status.typ.h"

#include <stddef.h> // for size_t

/**
 * @brief Allocate a frame.
 *
 * @note The returned pointer must be eventually released using free().
 *
 * @return Newly allocated frame.
 */
archi_hsp_frame_t*
archi_hsp_frame_alloc(
        size_t num_states ///< [in] Number of states in the frame.
);

/*****************************************************************************/

/**
 * @brief Get current state.
 *
 * If hsp is NULL, the function returns null state.
 *
 * @return Current state.
 */
archi_hsp_state_t
archi_hsp_current_state(
        archi_hsp_execution_context_t hsp ///< [in] Hierarchical state processor execution context.
);

/**
 * @brief Get number of frames on the HSP stack.
 *
 * If hsp is NULL, the function returns 0.
 *
 * @return Number of frames on the HSP stack.
 */
size_t
archi_hsp_stack_frames(
        archi_hsp_execution_context_t hsp ///< [in] Hierarchical state processor execution context.
);

/*****************************************************************************/

/**
 * @brief Get current state.
 *
 * This macro utilizes the parameter `hsp` implicitly provided by signature of ARCHI_HSP_STATE_FUNCTION().
 */
#define ARCHI_HSP_CURRENT_STATE() archi_hsp_current_state(hsp)

/**
 * @brief Get current number of frames on the stack.
 *
 * This macro utilizes the parameter `hsp` implicitly provided by signature of ARCHI_HSP_STATE_FUNCTION().
 */
#define ARCHI_HSP_STACK_FRAMES() archi_hsp_stack_frames(hsp)

/*****************************************************************************/

/**
 * @brief Proceed hierarchical state processor execution.
 *
 * This function pops the specified number of frames (sequences of states that are pushed at once),
 * then pushes a frame to the stack.
 * Null states in the frame are left out and not pushed.
 * Null metadata in pushed states is replaced with `pushed_frame.metadata`.
 *
 * num_popped_frames == 0: nothing is popped from the stack;
 * num_popped_frames == 1: the rest of the current frame is popped from the stack;
 * num_popped_frames >= 2: the rest of the current frame and (num_popped_frames - 1) frames are popped from the stack.
 *
 * If the stack is empty after the pop operation and there are no states pushed,
 * the hierarchical state processor exits.
 *
 * Returning from a state function normally is equivalent to calling ARCHI_HSP_FINISH(0).
 *
 * If hsp is NULL, the function does nothing.
 * If the function is not called from a state function during hierarchical state processor execution, it does nothing.
 * Otherwise, it does not return and triggers the state transition.
 */
void
archi_hsp_advance(
        archi_hsp_execution_context_t hsp, ///< [in] Hierarchical state processor execution context.

        size_t num_popped_frames, ///< [in] Number of frames to pop from the stack.

        size_t num_pushed_states, ///< [in] Number of states in the pushed frame.
        const archi_hsp_state_t *pushed_states ///< [in] States of the pushed frame.
);

/**
 * @brief Abort hierarchical state processor execution with the provided status code.
 *
 * This function is to be used to abort HSP execution in case of an error.
 *
 * If the code is 0, this function does nothing. To interrupt HSP execution normally,
 * pop all remaining frames with ARCHI_HSP_FINISH(ARCHI_HSP_STACK_FRAMES()).
 */
void
archi_hsp_abort(
        archi_hsp_execution_context_t hsp, ///< [in] Hierarchical state processor execution context.
        archi_status_t code ///< [in] Non-zero status code.
);

/*****************************************************************************/

/**
 * @brief Proceed hierarchical state processor execution.
 *
 * This macro utilizes the parameter `hsp` implicitly provided by signature of ARCHI_HSP_STATE_FUNCTION().
 */
#define ARCHI_HSP_ADVANCE(num_popped_frames, ...) do {                      \
    const archi_hsp_state_t _archi_hsp_pushed_states_[] = {__VA_ARGS__};    \
    archi_hsp_advance(hsp, (num_popped_frames),                             \
            sizeof(_archi_hsp_pushed_states_) / sizeof(archi_hsp_state_t),  \
            (_archi_hsp_pushed_states_));                                   \
} while (0)

/**
 * @brief Proceed hierarchical state processor execution without pushing a new frame.
 *
 * This macro utilizes the parameter `hsp` implicitly provided by signature of ARCHI_HSP_STATE_FUNCTION().
 */
#define ARCHI_HSP_FINISH(num_popped_frames) do {            \
    archi_hsp_advance(hsp, (num_popped_frames), 0, NULL);   \
} while (0)

/**
 * @brief Abort hierarchical state processor execution with the provided status code.
 *
 * This macro utilizes the parameter `hsp` implicitly provided by signature of ARCHI_HSP_STATE_FUNCTION().
 */
#define ARCHI_HSP_ABORT(code) do { \
    archi_hsp_abort(hsp, (code));  \
} while (0)

#endif // _ARCHI_HSP_API_STATE_FUN_H_

