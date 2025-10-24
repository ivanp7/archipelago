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
 * @brief Types for signal management.
 */

#pragma once
#ifndef _ARCHI_IPC_SIGNAL_API_MANAGEMENT_TYP_H_
#define _ARCHI_IPC_SIGNAL_API_MANAGEMENT_TYP_H_

#include "archi/ipc_signal/api/signal.typ.h"
#include "archi/ipc_signal/api/handler.typ.h"

struct archi_signal_management_context;

/**
 * @brief Pointer to signal management context.
 */
typedef struct archi_signal_management_context *archi_signal_management_context_t;

/**
 * @brief Parameters for starting the signal management subsystem (archi_signal_management_start()).
 *
 * This structure bundles the configuration needed to launch the
 * dedicated signal management thread. Once started:
 *   - The set of watched signals is fixed (signals->f_* flags).
 *   - The signal handler may be changed later, via archi_signal_management_set_handler().
 */
typedef struct archi_signal_management_start_params {
    archi_signal_watch_set_t *signals;     ///< Signals to watch.
    archi_signal_handler_t signal_handler; ///< Signal handler.
} archi_signal_management_start_params_t;

#endif // _ARCHI_IPC_SIGNAL_API_MANAGEMENT_TYP_H_

