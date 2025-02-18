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
 * @brief Finite state machine instance type.
 */

#pragma once
#ifndef _ARCHI_FSM_INSTANCE_TYP_H_
#define _ARCHI_FSM_INSTANCE_TYP_H_

#include "archi/fsm/state.typ.h"
#include "archi/fsm/transition.typ.h"

/**
 * @brief Finite state machine instance.
 */
typedef struct archi_fsm {
    archi_fsm_state_t entry_state;     ///< Entry state of the finite state machine.
    archi_fsm_transition_t transition; ///< State transition of the finite state machine.
} archi_fsm_t;

#endif // _ARCHI_FSM_INSTANCE_TYP_H_

