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
 * @brief Types for signal handlers.
 */

#pragma once
#ifndef _ARCHI_IPC_SIGNAL_API_HANDLER_TYP_H_
#define _ARCHI_IPC_SIGNAL_API_HANDLER_TYP_H_

#include <stdbool.h>

struct archi_signal_flags;

/**
 * @def ARCHI_SIGNAL_HANDLER_FUNC(func_name)
 * @brief Declare or define a signal handler function.
 *
 * This macro expands to the exact signature required for a signal
 * handler that the signal management subsystem will invoke whenever
 * a watched signal is caught.
 *
 * @note The handler runs in the context of the dedicated signal management thread
 * (not the async signal handler), so it may perform non-reentrant operations.
 * Signal handlers should avoid blocking operations to prevent delays in signal processing.
 *
 * @warning Never call exit(), quick_exit(), _Exit(), or similar
 *          process-terminating routines from inside a signal handler function,
 *          as it invokes undefined behavior.
 *
 * @param[in]       signo
 *                      Signal number (SIGINT, SIGTERM, etc).
 * @param[in]       siginfo
 *                      Pointer to the OS-provided siginfo_t.
 * @param[in,out]   flags
 *                      Pointer to the signal flags structure (archi_signal_flags_t).
 *                      The handler function is allowed to read and write states of any signals.
 * @param[in,out]   data
 *                      Signal handler function data.
 *
 * @return Whether the corresponding flag of the current signal should be set automatically.
 *   - true: the flag should be set;
 *   - false: the flag should not be set.
 *
 * @see archi_signal_handler_function_t
 */
#define ARCHI_SIGNAL_HANDLER_FUNC(func_name)    bool func_name( \
        int signo,                                              \
        void *const restrict siginfo,                           \
        struct archi_signal_flags *const restrict flags,        \
        void *const restrict data)

/**
 * @brief Function pointer type for a signal handler.
 *
 * Points to a function matching the ARCHI_SIGNAL_HANDLER_FUNC() signature.
 *
 * @see ARCHI_SIGNAL_HANDLER_FUNC
 */
typedef ARCHI_SIGNAL_HANDLER_FUNC((*archi_signal_handler_function_t));

/**
 * @struct archi_signal_handler_t
 * @brief Descriptor for a signal handler.
 *
 * Bundles a signal handler function pointer with an opaque user data pointer.
 * When a watched signal arrives, the signal manager invokes
 * handler->function(signo, siginfo, flags, handler->data).
 */
typedef struct archi_signal_handler {
    archi_signal_handler_function_t function; ///< Signal hander function.
    void *data; ///< Signal hander function data.
} archi_signal_handler_t;

#endif // _ARCHI_IPC_SIGNAL_API_HANDLER_TYP_H_

