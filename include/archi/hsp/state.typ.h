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
 * @brief Types for hierarchical state processor states.
 */

#pragma once
#ifndef _ARCHI_HSP_STATE_TYP_H_
#define _ARCHI_HSP_STATE_TYP_H_

#include <stddef.h>

struct archi_hsp_context;

/**
 * @brief Signature of a state function.
 *
 * @warning Use of variable-length arrays is not allowed in state functions,
 * as it will lead to memory leaks due to the hierarchical state processor implementation details.
 */
#define ARCHI_HSP_STATE_FUNCTION(name) void name( \
        struct archi_hsp_context *const hsp) /* Hierarchical state processor context. */

/**
 * @brief State function type.
 */
typedef ARCHI_HSP_STATE_FUNCTION((*archi_hsp_state_function_t));

/*****************************************************************************/

/**
 * @brief State of a hierarchical state processor.
 */
typedef struct archi_hsp_state {
    archi_hsp_state_function_t function; ///< State function.
    void *data; ///< State data.

    void *metadata; ///< State metadata for a transition function.
} archi_hsp_state_t;

/**
 * @brief Frame of a hierarchical state processor -- a sequence of states.
 */
typedef struct archi_hsp_frame {
    void *metadata; ///< Default state metadata for frame states.

    size_t num_states; ///< Number of states in the frame.
    archi_hsp_state_t state[]; ///< Frame states.
} archi_hsp_frame_t;

#endif // _ARCHI_HSP_STATE_TYP_H_

