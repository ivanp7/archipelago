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
 * @brief Types for finite state machine states.
 */

#pragma once
#ifndef _ARCHI_FSM_STATE_TYP_H_
#define _ARCHI_FSM_STATE_TYP_H_

struct archi_state_context;

/**
 * @brief State function.
 *
 * @see ARCHI_STATE_FUNCTION
 */
typedef void (*archi_state_function_t)(
        struct archi_state_context *const context ///< [in, out] State context.
);

/**
 * @brief State.
 */
typedef struct archi_state {
    archi_state_function_t function; ///< State function.
    void *data; ///< State data.
} archi_state_t;

/**
 * @brief Chain (linked list) of states.
 */
typedef struct archi_state_chain {
    void *data; ///< Current state data (or pointer to the next node).
    archi_state_t next_state; ///< Next state.
} archi_state_chain_t;

#endif // _ARCHI_FSM_STATE_TYP_H_

