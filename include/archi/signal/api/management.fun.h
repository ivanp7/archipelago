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
#ifndef _ARCHI_SIGNAL_API_MANAGEMENT_FUN_H_
#define _ARCHI_SIGNAL_API_MANAGEMENT_FUN_H_

#include "archi/signal/api/signal.typ.h"
#include "archi/signal/api/handler.typ.h"
#include "archi_base/global.typ.h"


/**
 * @brief Initialize the signal management context pointer.
 *
 * @pre
 *      context != NULL
 *
 * @post
 *      Signal management API is ready to be used.
 *
 * @note Subsequent invocations have no effect.
 *
 * @param[in] context
 *      Signal management context.
 */
ARCHI_GLOBAL_SET_FUNC(archi_global_context_set__signal_management);

/**
 * @brief Retrieve the signal management context pointer.
 *
 * If the signal management context pointer hasn't been initialized yet, the function returns NULL.
 *
 * @return Pointer to the signal management context.
 */
ARCHI_GLOBAL_GET_FUNC(archi_global_context__signal_management);

/**
 * @brief Retrieve the set of signal flags.
 *
 * Provides access to the atomic flag structure that reflects which
 * signals have been caught since the last reset of flags.
 *
 * @note
 *   May be called from any thread to inspect or clear signal flags.
 *
 * @return
 *   Pointer to the archi_signal_flags_t object
 *   if the signal management thread is running; otherwise NULL.
 */
archi_signal_flags_t*
archi_signal_management_flags(void);

/**
 * @brief Get the currently installed signal handler.
 *
 * Retrieves the archi_signal_handler_t stored in the context.
 *
 * @note
 *   May be called from any thread. Internally protected by a mutex.
 *
 * @return
 *   The current archi_signal_handler_t if the signal management thread is running,
 *   otherwise returns a zero‑initialized handler.
 */
archi_signal_handler_t
archi_signal_management_handler(void);

/**
 * @brief Install, replace, or uninstall the signal handler.
 *
 * Updates the archi_signal_handler_t that will be used whenever the
 * management thread catches a watched signal.
 *
 * @note
 *   May be called from any thread. Internally protected by a mutex.
 *
 * @param[in] signal_handler
 *   Signal handler to install. If signal_handler.function is NULL,
 *   the current handler is uninstalled without replacement.
 */
void
archi_signal_management_handler_set(
        archi_signal_handler_t signal_handler
);

#endif // _ARCHI_SIGNAL_API_MANAGEMENT_FUN_H_

