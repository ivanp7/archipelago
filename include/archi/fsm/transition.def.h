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
 * @brief Constants and macros for implementation of plugin state transitions.
 */

#pragma once
#ifndef _ARCHI_FSM_TRANSITION_DEF_H_
#define _ARCHI_FSM_TRANSITION_DEF_H_

#include "archi/fsm/state.typ.h"

/**
 * @brief Null (empty) state transition.
 */
#define ARCHI_NULL_STATE_TRANSITION (archi_state_transition_t){0}

/**
 * @brief State transition.
 */
#define ARCHI_STATE_TRANSITION(func, data_ptr) \
    (archi_state_transition_t){.function = (func), .data = (data_ptr)}

/*****************************************************************************/

/**
 * @brief Declarator of a state transition function.
 *
 * @see archi_state_transition_function_t
 */
#define ARCHI_STATE_TRANSITION_FUNCTION(name) void name(    \
        const archi_state_t prev_state,                     \
        archi_state_t *const restrict next_state,           \
        archi_state_t *const restrict return_state,         \
        void *const restrict data)

#endif // _ARCHI_FSM_TRANSITION_DEF_H_

