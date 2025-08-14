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
 * @brief Operations on signal sets.
 */

#pragma once
#ifndef _ARCHI_IPC_SIGNAL_API_SIGNAL_FUN_H_
#define _ARCHI_IPC_SIGNAL_API_SIGNAL_FUN_H_

#include "archi/ipc_signal/api/signal.typ.h"
#include "archipelago/util/size.def.h"

#include <stddef.h>

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
 * @brief Test if a signal watch set is not empty.
 *
 * If any of the signal watch set fields is set to true, the function returns true.
 * Otherwise, it returns false.
 * If the pointer is NULL, the function returns false.
 *
 * @param[in] signals
 *   Pointer to the watch set.
 */
bool
archi_signal_watch_set_not_empty(
        const archi_signal_watch_set_t *signals
);

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

#endif // _ARCHI_IPC_SIGNAL_API_SIGNAL_FUN_H_

