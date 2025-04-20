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
 * @brief Signal management operations.
 */

#pragma once
#ifndef _ARCHI_IPC_SIGNAL_INTERFACE_FUN_H_
#define _ARCHI_IPC_SIGNAL_INTERFACE_FUN_H_

#include "archi/ipc/signal/interface.typ.h"
#include "archi/util/status.typ.h"
#include "archi/util/size.def.h"

#include <stddef.h>

struct archi_signal_management_context;

/**
 * @brief Retrieve the count of POSIX real‑time signals supported by the system.
 *
 * POSIX real‑time signals occupy the continuous range
 * [SIGRTMIN ... SIGRTMAX]. This function computes the total number
 * of distinct real‑time signals available on the current platform.
 *
 * @return
 *   (SIGRTMAX - SIGRTMIN + 1), i.e. the size of the real‑time signal range.
 */
size_t
archi_signal_number_of_rt_signals(void);

/**
 * @brief Merge two signal watch sets by logical OR.
 *
 * For each signal in the standard and real-time ranges, this function
 * updates the "out" set to include any signal that is watched in
 * either the "out" or "in" set. In effect:
 *   out->f_SIGNAL = out->f_SIGNAL  || in->f_SIGNAL
 * for every supported signal SIGNAL.
 *
 * If either pointer is NULL, the function does nothing.
 *
 * @param[in,out] out
 *   Pointer to the first watch set, which will be updated in place.
 *
 * @param[in] in
 *   Pointer to the second watch set, whose flags are merged into "out".
 */
void
archi_signal_watch_set_join(
        archi_signal_watch_set_t *out,
        const archi_signal_watch_set_t *in
);

/**
 * @brief Allocate and initialize a new signal watch set.
 *
 * This function allocates a zero-initialized archi_signal_watch_set_t
 * structure whose size is equal to ARCHI_SIGNAL_WATCH_SET_SIZEOF.
 * All signal watch flags (standard and real‑time) are cleared (false).
 *
 * @note The returned pointer must eventually be released via free().
 *
 * @return
 *   Pointer to a freshly allocated and initialized signal watch set,
 *   or NULL if memory allocation fails.
 */
archi_signal_watch_set_t*
archi_signal_watch_set_alloc(void);

/**
 * @brief Allocate and initialize a new signal flags structure.
 *
 * This function allocates an archi_signal_flags_t structure of size
 * ARCHI_SIGNAL_FLAGS_SIZEOF and initializes each per-signal flag via
 * the ARCHI_SIGNAL_INIT_FLAG macro (initializing atomics to false).
 *
 * @note The returned pointer must eventually be released via free().
 *
 * @return
 *   Pointer to a freshly allocated and initialized signal flags structure,
 *   or NULL if memory allocation fails.
 */
archi_signal_flags_t*
archi_signal_flags_alloc(void);

/*****************************************************************************/

/**
 * @brief Size of signal watch set structure in bytes.
 */
#define ARCHI_SIGNAL_WATCH_SET_SIZEOF   ARCHI_SIZEOF_FLEXIBLE( \
        archi_signal_watch_set_t, f_SIGRTMIN, archi_signal_number_of_rt_signals())

/**
 * @brief Size of signal flags structure in bytes.
 */
#define ARCHI_SIGNAL_FLAGS_SIZEOF       ARCHI_SIZEOF_FLEXIBLE( \
        archi_signal_flags_t, f_SIGRTMIN, archi_signal_number_of_rt_signals())

/*****************************************************************************/

/**
 * @def ARCHI_SIGNAL_IS_FLAG_SET(flag)
 * @brief Atomically read the value of a signal flag.
 *
 * @param flag
 *   An atomic_bool lvalue to test.
 * @return
 *   true if the flag is set, false otherwise.
 */
#define ARCHI_SIGNAL_IS_FLAG_SET(flag)  atomic_load_explicit(&(flag), memory_order_relaxed)

/**
 * @def ARCHI_SIGNAL_INIT_FLAG(flag)
 * @brief Initialize a signal flag to false.
 *
 * Uses atomic_init to configure the atomic_bool storage.
 * Must be called before any other operations on @p flag.
 *
 * @param flag
 *   An atomic_bool lvalue to initialize.
 */
#define ARCHI_SIGNAL_INIT_FLAG(flag)    do { \
    atomic_init(&(flag), false); \
} while (0)

/**
 * @def ARCHI_SIGNAL_SET_FLAG(flag)
 * @brief Atomically set a signal flag (to true).
 *
 * @param flag
 *   An atomic_bool lvalue to set.
 */
#define ARCHI_SIGNAL_SET_FLAG(flag)     do { \
    atomic_store_explicit(&(flag), true, memory_order_relaxed); \
} while (0)

/**
 * @def ARCHI_SIGNAL_UNSET_FLAG(flag)
 * @brief Atomically clear a signal flag (to false).
 *
 * @param flag
 *   An atomic_bool lvalue to clear.
 */
#define ARCHI_SIGNAL_UNSET_FLAG(flag)   do { \
    atomic_store_explicit(&(flag), false, memory_order_relaxed); \
} while (0)

/*****************************************************************************/

/**
 * @brief Initialize and start the signal management thread.
 *
 * This function creates and starts a dedicated signal management thread
 * that monitors and handles POSIX signals. The thread runs asynchronously
 * and dispatches signals to the registered handler
 * (and sets the corresponding flags unless the handler returns false).
 *
 * @warning
 *   - Only one signal management thread should exist per process.
 *   - This function must be called in the main thread before any other
 *     threads are created to ensure proper signal handling.
 *
 * @pre
 *   - params.signals != NULL
 *
 * @post
 *   - A dedicated signal management thread is created and running.
 *   - All specified signals are blocked in the main thread and will be handled
 *     by the signal management thread.
 *   - The provided signal handler (if any) will be invoked for the watched signals.
 *
 * @param[in] params
 *   Configuration parameters specifying:
 *     - The set of signals to watch (params.signals)
 *     - An optional initial signal handler (params.signal_handler)
 *
 * @param[out] code
 *   Status code:
 *     - 0:                         success;
 *     - ARCHI_STATUS_EMISUSE:      invalid arguments (NULL pointers);
 *     - ARCHI_STATUS_ENOMEMORY:    memory allocation failure;
 *     - ARCHI_STATUS_ERESOURCE:    failure to create thread or block signals.
 *
 * @return
 *   Pointer to the signal management context on success.
 *   NULL on failure (e.g., unable to allocate memory or create thread).
 *
 * @note
 *   - The signal management context and associated thread must be properly
 *     destroyed using archi_signal_management_stop().
 */
struct archi_signal_management_context*
archi_signal_management_start(
        archi_signal_management_start_params_t params,
        archi_status_t *code
);

/**
 * @brief Stop the signal management thread and clean up.
 *
 * This function terminates the dedicated signal management thread,
 * unblocks all signals that were being watched, and frees all
 * resources associated with the given context.
 *
 * @note
 *   - Safe to call with a NULL pointer: no action is performed.
 *   - This call blocks until the signal thread has fully exited.
 *
 * @warning
 *   - Do not attempt to stop the thread from within a signal handler function.
 *
 * @pre
 *   - @p context was previously created by archi_signal_management_start().
 *
 * @post
 *   - If the signal management thread was running:
 *       • The thread is joined (blocks until the thread exits).
 *       • The process mask is restored (watched signals are unblocked).
 *   - The context structure and associated resources are freed.
 *   - Any use of @p context or its flag pointers after this call is invalid.
 *
 * @param[in,out] context
 *   Pointer to the signal management context returned by
 *   archi_signal_management_start(). If NULL, this function does nothing.
 */
void
archi_signal_management_stop(
        struct archi_signal_management_context *context
);

/**
 * @brief Retrieve the set of signal flags.
 *
 * Provides access to the atomic flag structure that reflects which
 * signals have been caught since the last reset.
 *
 * @note
 *   May be called from any thread to inspect or clear signal flags.
 *
 * @pre
 *   - @p context was returned by archi_signal_management_start() and has
 *     not yet been stopped.
 *
 * @param[in] context
 *   Pointer to a valid signal management context.
 *
 * @return
 *   Pointer to the context’s archi_signal_flags_t if @p context is not NULL;
 *   otherwise NULL.
 */
archi_signal_flags_t*
archi_signal_management_flags(
        struct archi_signal_management_context *context
);

/**
 * @brief Get the currently installed signal handler.
 *
 * Retrieves the archi_signal_handler_t stored in the context.
 *
 * @note
 *   May be called from any thread. Internally protected by an atomic spinlock.
 *
 * @pre
 *   - @p context was returned by archi_signal_management_start() and has
 *     not yet been stopped.
 *
 * @param[in] context
 *   Pointer to a valid signal management context.
 *
 * @return
 *   The current archi_signal_handler_t. If @p context is NULL, returns
 *   a zero‑initialized handler (function == NULL).
 */
archi_signal_handler_t
archi_signal_management_handler(
        struct archi_signal_management_context *context
);

/**
 * @brief Install, replace, or uninstall the signal handler.
 *
 * Updates the archi_signal_handler_t that will be invoked whenever the
 * management thread catches a watched signal.
 *
 * @note
 *   May be called from any thread. Internally protected by an atomic spinlock.
 *
 * @pre
 *   - @p context was returned by archi_signal_management_start() and has
 *     not yet been stopped.
 *
 * @param[in,out] context
 *   Pointer to a valid signal management context.
 *
 * @param[in] signal_handler
 *   Signal handler to install. If signal_handler.function is NULL,
 *   the current handler is uninstalled without replacement.
 */
void
archi_signal_management_set_handler(
        struct archi_signal_management_context *context,
        archi_signal_handler_t signal_handler
);

#endif // _ARCHI_IPC_SIGNAL_INTERFACE_FUN_H_

