/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
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
 * @brief Signal management operations.
 */

#pragma once
#ifndef _ARCHI_APP_SIGNAL_MANAGEMENT_FUN_H_
#define _ARCHI_APP_SIGNAL_MANAGEMENT_FUN_H_

#include "archi/signal/api/signal.typ.h"
#include "archi/signal/api/handler.typ.h"
#include "archi_base/global.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Retrieve the signal management context.
 *
 * @return Pointer to the signal management context.
 */
ARCHI_GLOBAL_GET_FUNC(archi_app_signal_management_global_context);

/**
 * @brief Initialize and start the signal management thread.
 *
 * This function creates and starts a dedicated signal management thread
 * that monitors and handles POSIX signals. The thread runs asynchronously
 * and dispatches signals to the registered handler
 * (and sets the corresponding flags unless the handler returns false).
 *
 * @warning
 *   - Only one signal management thread can exist per process.
 *   - This function must be called in the main thread before any other
 *     threads are created to ensure proper signal handling.
 *
 * Once started:
 *   - The set of watched signals is fixed (signals->f_* flags).
 *   - The signal handler may be changed later, via archi_signal_management_set_handler().
 *
 * @pre
 *   - signals != NULL
 *
 * @post
 *   - A dedicated signal management thread is created and running.
 *   - All specified signals are blocked in the main thread and will be handled
 *     by the signal management thread.
 *   - The provided signal handler (if any) will be invoked for the watched signals.
 *
 * @note
 *   - The signal management context and associated thread must be properly
 *     stopped using archi_signal_management_stop().
 */
void
archi_app_signal_management_start(
        archi_signal_set_const_t signals, ///< [in] Signals to watch.
        archi_signal_handler_t signal_handler, ///< [in] Signal handler.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Stop the signal management thread and clean up.
 *
 * This function terminates the dedicated signal management thread,
 * unblocks all signals that were watched, and frees all
 * resources associated with the signal management.
 *
 * @note
 *   - This call blocks until the signal thread has fully exited.
 *
 * @warning
 *   - Do not attempt to stop the thread from within a signal handler function.
 *
 * @post
 *   - If the signal management thread was running:
 *       + The thread is joined (blocks until the thread exits).
 *       + The process mask is restored (watched signals are unblocked).
 *   - The associated resources are freed.
 *   - The signal flags structure is invalid after this call.
 */
void
archi_app_signal_management_stop(void);

#endif // _ARCHI_APP_SIGNAL_MANAGEMENT_FUN_H_

