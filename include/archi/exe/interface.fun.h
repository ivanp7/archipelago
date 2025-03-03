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
 * @brief Application context interface.
 */

#pragma once
#ifndef _ARCHI_EXE_INTERFACE_FUN_H_
#define _ARCHI_EXE_INTERFACE_FUN_H_

#include "archi/app/context.typ.h"

ARCHI_CONTEXT_SET_FUNC(archi_app_signal_set); ///< Signal management context slot setter function.
ARCHI_CONTEXT_GET_FUNC(archi_app_signal_get); ///< Signal management context slot getter function.

extern
const archi_context_interface_t archi_app_signal_interface; ///< Signal management context interface.

#define ARCHI_APP_SIGNAL_SLOT_SIGNAL_HANDLER "signal_handler" ///< Input slot: signal handler.
#define ARCHI_APP_SIGNAL_SLOT_SIGNAL_HANDLER_FUNC "signal_handler_function" ///< Input/output slot: signal handler function.
#define ARCHI_APP_SIGNAL_SLOT_SIGNAL_HANDLER_DATA "signal_handler_data" ///< Input/output slot: signal handler data.

#define ARCHI_APP_SIGNAL_SLOT_SIGNAL_FLAGS "signal_flags" ///< Output slot: signal flags.

#define ARCHI_APP_SIGNAL_CONTEXT "archi_app_signal" ///< Key of the application signal management context.

/*****************************************************************************/

ARCHI_CONTEXT_SET_FUNC(archi_app_fsm_set); ///< FSM context slot setter function.
ARCHI_CONTEXT_GET_FUNC(archi_app_fsm_get); ///< FSM context slot getter function.

extern
const archi_context_interface_t archi_app_fsm_interface; ///< FSM context interface.

#define ARCHI_APP_FSM_SLOT_ENTRY_STATE "entry_state" ///< Input/output slot: entry state.
#define ARCHI_APP_FSM_SLOT_ENTRY_STATE_FUNC "entry_state_function" ///< Input/output slot: entry state function.
#define ARCHI_APP_FSM_SLOT_ENTRY_STATE_DATA "entry_state_data" ///< Input/output slot: entry state data.
#define ARCHI_APP_FSM_SLOT_ENTRY_STATE_METADATA "entry_state_metadata" ///< Input/output slot: entry state metadata.

#define ARCHI_APP_FSM_SLOT_TRANSITION "transition" ///< Input/output slot: state transition.
#define ARCHI_APP_FSM_SLOT_TRANSITION_FUNC "transition_function" ///< Input/output slot: state transition function.
#define ARCHI_APP_FSM_SLOT_TRANSITION_DATA "transition_data" ///< Input/output slot: state transition data.

#define ARCHI_APP_FSM_CONTEXT "archi_app_fsm" ///< Key of the application FSM context.

#endif // _ARCHI_EXE_INTERFACE_FUN_H_

