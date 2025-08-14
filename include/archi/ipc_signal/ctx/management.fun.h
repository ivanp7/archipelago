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
 * @brief Application context interface functions for the signal management.
 */

#pragma once
#ifndef _ARCHI_IPC_SIGNAL_CTX_MANAGEMENT_FUN_H_
#define _ARCHI_IPC_SIGNAL_CTX_MANAGEMENT_FUN_H_

#include "archipelago/context/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_ipc_signal_management_init);   ///< Signal management context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_ipc_signal_management_final); ///< Signal management context finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_ipc_signal_management_get);     ///< Signal management context slot getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_ipc_signal_management_set);     ///< Signal management context slot setter function.

// No interface is provided, as the signal management thread has the tight restrictions on its use.
// There can be only one signal management thread, and it must be created before any other threads.
// Thus, it has to be controlled by an application in a special way.

#endif // _ARCHI_IPC_SIGNAL_CTX_MANAGEMENT_FUN_H_

