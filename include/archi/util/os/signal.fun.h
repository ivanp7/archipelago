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
 * @brief Signal management operations.
 */

#pragma once
#ifndef _ARCHI_UTIL_OS_SIGNAL_FUN_H_
#define _ARCHI_UTIL_OS_SIGNAL_FUN_H_

#include "archi/util/os/signal.typ.h"

#include <stddef.h>

struct archi_signal_management_context;

/**
 * @brief Get number of supported real-time signals.
 *
 * @return Number of supported real-time signals.
 */
size_t
archi_signal_number_of_rt_signals(void);

/**
 * @brief Allocate and initialize signal watch set structure.
 *
 * The pointer must be deallocated with free().
 *
 * @return Signal watch set.
 */
archi_signal_watch_set_t*
archi_signal_watch_set_alloc(void);

/**
 * @brief Allocate and initialize signal flags structure.
 *
 * The pointer must be deallocated with free().
 *
 * @return Signal flags.
 */
archi_signal_flags_t*
archi_signal_flags_alloc(void);

/*****************************************************************************/

/**
 * @brief Start signal management thread.
 *
 * @warning There should be only one signal management thread per application,
 * which must be created in the main thread before any other thread,
 * otherwise signal management won't work (reliably or at all).
 *
 * Input set of signals specifies which signals are to be watched.
 *
 * Before the thread is started, all flags are cleared (reset to false).
 * When a signal is caught, its corresponding flag is set to true.
 *
 * Signal handler argument is optional.
 * This handler is called synchronously and used to get access to the siginfo_t value.
 *
 * @return Signal management context.
 */
struct archi_signal_management_context*
archi_signal_management_thread_start(
        const archi_signal_management_config_t *config ///< [in] Signal management configuration.
);

/**
 * @brief Stop signal management thread.
 */
void
archi_signal_management_thread_stop(
        struct archi_signal_management_context *context ///< [in] Signal management context.
);

/**
 * @brief Extract signal management thread properties.
 */
void
archi_signal_management_thread_get_properties(
        struct archi_signal_management_context *context, ///< [in] Signal management context.

        archi_signal_flags_t **flags, ///< [out] Place for pointer to signal flags.
        archi_signal_handler_t *signal_handler ///< [out] Place for signal handler.
);

#endif // _ARCHI_UTIL_OS_SIGNAL_FUN_H_

