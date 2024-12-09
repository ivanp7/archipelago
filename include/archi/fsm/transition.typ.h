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
 * @brief Types for finite state machine transitions.
 */

#pragma once
#ifndef _ARCHI_FSM_TRANSITION_TYP_H_
#define _ARCHI_FSM_TRANSITION_TYP_H_

#include "archi/fsm/state.typ.h"
#include "archi/util/status.typ.h"

/**
 * @brief State transition function.
 *
 * @see ARCHI_STATE_TRANSITION_FUNCTION
 */
typedef void (*archi_state_transition_function_t)(
        const archi_state_t prev_state, ///< [in] Previous state.
        const archi_state_t next_state, ///< [in] Next state.

        archi_state_t *const restrict trans_state, ///< [out] Transitional state.

        archi_status_t *const code, ///< [in,out] Status code.

        void *const restrict data ///< [in,out] State transition data.
);

/**
 * @brief State transition.
 */
typedef struct archi_state_transition {
    archi_state_transition_function_t function; ///< State transition function.
    void *data; ///< State transition data.
} archi_state_transition_t;

#endif // _ARCHI_FSM_TRANSITION_TYP_H_

