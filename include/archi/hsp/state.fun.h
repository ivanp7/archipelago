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
#ifndef _ARCHI_HSP_STATE_FUN_H_
#define _ARCHI_HSP_STATE_FUN_H_

#include "archi/hsp/state.typ.h"
#include "archi/util/status.typ.h"

#include <stddef.h>

/**
 * @brief Get current state.
 *
 * If hsp is NULL, the function returns null state.
 *
 * @return Current state.
 */
archi_hsp_state_t
archi_hsp_current_state(
        const struct archi_hsp_context *hsp ///< [in] Hierarchical state processor context.
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
        const struct archi_hsp_context *hsp ///< [in] Hierarchical state processor context.
);

/*****************************************************************************/

/**
 * @brief Get current state.
 *
 * This macro utilizes the parameter `hsp` implicitly provided by signature of ARCHI_HSP_STATE_FUNCTION().
 */
#define ARCHI_HSP_CURRENT_STATE() archi_hsp_current_state(hsp)

/**
 * @brief Access current number of frames on the stack.
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
 * num_pop_frames == 0: nothing is popped from the stack;
 * num_pop_frames == 1: the rest of the current frame is popped from the stack;
 * num_pop_frames >= 2: the rest of the current frame and (num_pop_frames - 1) frames are popped from the stack.
 *
 * If the stack is empty after the pop operation and there are no states pushed,
 * the hierarchical state processor exits.
 *
 * Returning from a state function normally is equivalent to calling
 * archi_hsp_advance(hsp, 0, (archi_hsp_frame_t){0});
 *
 * If hsp is NULL, the function does nothing.
 * If the function is not called from a state function during hierarchical state processor execution, it does nothing.
 * Otherwise, it does not return and triggers the state transition.
 */
void
archi_hsp_advance(
        struct archi_hsp_context *hsp, ///< [in] Hierarchical state processor context.

        size_t num_pop_frames, ///< [in] Number of frames to pop from the stack.
        archi_hsp_frame_t pushed_frame ///< [in] Frame to push to the stack.
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
        struct archi_hsp_context *hsp, ///< [in] Hierarchical state processor context.
        archi_status_t code ///< [in] Non-zero status code.
);

/*****************************************************************************/

/**
 * @brief Proceed hierarchical state processor execution.
 *
 * This macro utilizes the parameter `hsp` implicitly provided by signature of ARCHI_HSP_STATE_FUNCTION().
 */
#define ARCHI_HSP_ADVANCE(num_pop_frames, ...) do {                 \
    const archi_hsp_state_t _archi_frame_states_[] = {__VA_ARGS__}; \
    archi_hsp_advance(hsp, (num_pop_frames), (archi_hsp_frame_t){   \
            .state = _archi_frame_states_,                          \
            .num_states = sizeof(_archi_frame_states_) /            \
                          sizeof(_archi_frame_states_[0]),          \
            .metadata = archi_hsp_current_state(hsp).metadata});    \
} while (0)

/**
 * @brief Proceed hierarchical state processor execution without pushing a new frame.
 *
 * This macro utilizes the parameter `hsp` implicitly provided by signature of ARCHI_HSP_STATE_FUNCTION().
 */
#define ARCHI_HSP_FINISH(num_pop_frames) do { \
    archi_hsp_advance(hsp, (num_pop_frames), (archi_hsp_frame_t){0}); \
} while (0)

/**
 * @brief Abort hierarchical state processor execution with the provided status code.
 *
 * This macro utilizes the parameter `hsp` implicitly provided by signature of ARCHI_HSP_STATE_FUNCTION().
 */
#define ARCHI_HSP_ABORT(code) do { \
    archi_hsp_abort(hsp, (code));  \
} while (0)

#endif // _ARCHI_HSP_STATE_FUN_H_

