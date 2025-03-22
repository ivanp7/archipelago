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
 * @brief Types for finite state machine transitions.
 */

#pragma once
#ifndef _ARCHI_FSM_TRANSITION_TYP_H_
#define _ARCHI_FSM_TRANSITION_TYP_H_

#include "archi/fsm/state.typ.h"

/**
 * @brief Declarator of a state transition function.
 */
#define ARCHI_FSM_TRANSITION_FUNCTION(name) void name( \
        const archi_fsm_state_t prev_state, /* Previous state. */ \
        const archi_fsm_state_t next_state, /* Next state. */ \
        archi_fsm_state_t *const restrict trans_state, /* Transitional state. */ \
        void *const restrict data) /* State transition data. */

/**
 * @brief State transition function.
 */
typedef ARCHI_FSM_TRANSITION_FUNCTION((*archi_fsm_transition_function_t));

/*****************************************************************************/

/**
 * @brief State transition of a finite state machine.
 */
typedef struct archi_fsm_transition {
    archi_fsm_transition_function_t function; ///< State transition function.
    void *data; ///< State transition data.
} archi_fsm_transition_t;

/**
 * @brief Null (empty) state transition.
 */
#define ARCHI_NULL_FSM_TRANSITION (archi_fsm_transition_t){0}

/**
 * @brief State transition.
 */
#define ARCHI_FSM_TRANSITION(func, data_ptr) \
    (archi_fsm_transition_t){.function = (func), .data = (data_ptr)}

#endif // _ARCHI_FSM_TRANSITION_TYP_H_

