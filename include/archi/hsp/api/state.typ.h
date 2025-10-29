/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
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
#ifndef _ARCHI_HSP_API_STATE_TYP_H_
#define _ARCHI_HSP_API_STATE_TYP_H_

#include "archi/hsp/api/exec.typ.h"
#include "archipelago/base/error.typ.h"

#include <stddef.h> // for size_t

/**
 * @brief Signature of a state function.
 *
 * @warning Use of variable-length arrays is not allowed in state functions,
 * as it will lead to memory leaks due to the hierarchical state processor implementation details.
 */
#define ARCHI_HSP_STATE_FUNCTION(name) void name(                               \
        void *data, /* [in] Current state data. */                              \
        archi_hsp_execution_context_t hsp, /* [in] HSP execution context. */    \
        ARCHI_ERROR_PARAMETER_DECL) /* [out] Error. */

/**
 * @brief State function type.
 */
typedef ARCHI_HSP_STATE_FUNCTION((*archi_hsp_state_function_t));

/**
 * @brief Function type tag for state functions.
 */
#define ARCHI_POINTER_FUNCTION_TAG__HSP_STATE   2

/*****************************************************************************/

/**
 * @brief State of a hierarchical state processor.
 */
typedef struct archi_hsp_state {
    archi_hsp_state_function_t function; ///< State function.
    void *data; ///< State data.

    void *metadata; ///< State metadata for a transition function use.
} archi_hsp_state_t;

/**
 * @brief Frame of a hierarchical state processor -- a sequence of states.
 */
typedef struct archi_hsp_frame {
    const size_t num_states;   ///< Number of states in the frame.
    archi_hsp_state_t state[]; ///< Frame states.
} archi_hsp_frame_t;

#endif // _ARCHI_HSP_API_STATE_TYP_H_

