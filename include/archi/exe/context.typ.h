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
 * @brief Application context type.
 */

#pragma once
#ifndef _ARCHI_EXE_CONTEXT_TYP_H_
#define _ARCHI_EXE_CONTEXT_TYP_H_

#include "archi/fsm/state.typ.h"
#include "archi/fsm/transition.typ.h"
#include "archi/app/context.typ.h"
#include "archi/util/container.typ.h"
#include "archi/util/list.typ.h"
#include "archi/util/os/signal.typ.h"

#include <stdatomic.h> // for atomic_flag

/**
 * @brief Context of an application.
 */
typedef struct archi_app_context {
    archi_state_t entry_state;     ///< Entry state of the finite state machine.
    archi_transition_t transition; ///< State transition of the finite state machine.

    archi_signal_handler_t signal_handler; ///< Signal handler.
    archi_signal_flags_t *signal_flags;    ///< Signal flags.
} archi_app_context_t;

#define ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE "entry_state" ///< Input/output slot: entry state.
#define ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE_FUNC "entry_state.function" ///< Input/output slot: entry state function.
#define ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE_DATA "entry_state.data" ///< Input/output slot: entry state data.
#define ARCHI_APP_CONTEXT_SLOT_ENTRY_STATE_METADATA "entry_state.metadata" ///< Input/output slot: entry state metadata.

#define ARCHI_APP_CONTEXT_SLOT_TRANSITION "transition" ///< Input/output slot: state transition.
#define ARCHI_APP_CONTEXT_SLOT_TRANSITION_FUNC "transition.function" ///< Input/output slot: state transition function.
#define ARCHI_APP_CONTEXT_SLOT_TRANSITION_DATA "transition.data" ///< Input/output slot: state transition data.

#define ARCHI_APP_CONTEXT_SLOT_SIGNAL_HANDLER "signal.handler" ///< Input/output slot: signal handler.
#define ARCHI_APP_CONTEXT_SLOT_SIGNAL_HANDLER_FUNC "signal.handler.function" ///< Input/output slot: signal handler function.
#define ARCHI_APP_CONTEXT_SLOT_SIGNAL_HANDLER_DATA "signal.handler.data" ///< Input/output slot: signal handler data.

#define ARCHI_APP_CONTEXT_SLOT_SIGNAL_FLAGS "signal.flags" ///< Output slot: signal flags.

/*****************************************************************************/

/**
 * @brief Signal handler protected by spinlock.
 */
typedef struct archi_app_signal_handler_spinlock {
    archi_signal_handler_t signal_handler; ///< Signal handler.
    atomic_flag spinlock; ///< Spinlock protecting the signal handler.
} archi_app_signal_handler_spinlock_t;

/**
 * @brief An application.
 */
typedef struct archi_application {
    archi_app_context_t context;                ///< Application context.
    archi_context_t context_handle_interface;   ///< Application context handle with interface.
    archi_list_node_named_value_t context_node; ///< List node for the app context.

    struct {
        archi_list_container_data_t container_data; ///< List of the container.
        archi_container_t container;                ///< Container itself.
    } plugin_contexts,              ///< Container of pointers to plugin contexts.
        plugin_context_interfaces,  ///< Container of pointers to plugin context interfaces.
        plugin_libraries;           ///< Container of handles plugin shared libraries.

    struct archi_signal_management_context *signal_management;   ///< Signal management context.
    archi_app_signal_handler_spinlock_t signal_handler_spinlock; ///< Signal handler protected by spinlock.
} archi_application_t;

#endif // _ARCHI_EXE_CONTEXT_TYP_H_

