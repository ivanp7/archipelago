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
 * @brief Types for hierarchical state processor transitions.
 */

#pragma once
#ifndef _ARCHI_HSP_API_TRANSITION_TYP_H_
#define _ARCHI_HSP_API_TRANSITION_TYP_H_

#include "archi/hsp/api/state.typ.h"

/**
 * @brief Signature of a state transition function.
 */
#define ARCHI_HSP_TRANSITION_FUNCTION(name) void name( \
        const archi_hsp_state_t prev_state, /* [in] Previous state. */ \
        const archi_hsp_state_t next_state, /* [in] Next state. */ \
        archi_hsp_state_t *const restrict trans_state, /* [out] (optional) Transitional state. */ \
        void *const restrict data) /* [in] State transition data. */

/**
 * @brief State transition function type.
 */
typedef ARCHI_HSP_TRANSITION_FUNCTION((*archi_hsp_transition_function_t));

/*****************************************************************************/

/**
 * @brief State transition of a hierarchical state processor.
 */
typedef struct archi_hsp_transition {
    archi_hsp_transition_function_t function; ///< State transition function.
    void *data; ///< State transition data.
} archi_hsp_transition_t;

#endif // _ARCHI_HSP_API_TRANSITION_TYP_H_

