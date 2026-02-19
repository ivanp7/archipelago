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
 * @brief Operations on signal sets.
 */

#pragma once
#ifndef _ARCHI_SIGNAL_API_SIGNAL_FUN_H_
#define _ARCHI_SIGNAL_API_SIGNAL_FUN_H_

#include "archi/signal/api/signal.typ.h"

#include <stdbool.h>


/**
 * @brief Allocate and initialize a new signal set.
 *
 * This function allocates a archi_signal_set_t object.
 * All signal flags are initialized to clear state.
 *
 * @note The returned pointer must eventually be released via free().
 *
 * @return
 *   Pointer to a freshly allocated and initialized signal set,
 *   or NULL if memory allocation fails.
 */
archi_signal_set_t
archi_signal_set_alloc(void);

/**
 * @brief Check if a signal set is empty.
 *
 * @return True of set is NULL or empty, false otherwise.
 */
bool
archi_signal_set_is_empty(
        archi_signal_set_const_t set ///< [in] Signal set.
);

/**
 * @brief Remove all signals from a set.
 *
 * If set is NULL, the function does nothing.
 */
void
archi_signal_set_clear(
        archi_signal_set_t set ///< [in] Signal set.
);

/**
 * @brief Invert a signal set.
 *
 * If set is NULL, the function does nothing.
 */
void
archi_signal_set_invert(
        archi_signal_set_t set ///< [in] Signal set.
);

/**
 * @brief Assign a signal set to another.
 *
 * If either set is NULL, the function does nothing.
 */
void
archi_signal_set_assign(
        archi_signal_set_t out, ///< [in,out] First signal set.
        archi_signal_set_const_t in ///< [in] Second signal set.
);

/**
 * @brief Join two signal sets into the first argument.
 *
 * If either set is NULL, the function does nothing.
 */
void
archi_signal_set_join(
        archi_signal_set_t out, ///< [in,out] First signal set.
        archi_signal_set_const_t in ///< [in] Second signal set.
);

/**
 * @brief Intersect two signal sets into the first argument.
 *
 * If either set is NULL, the function does nothing.
 */
void
archi_signal_set_intersect(
        archi_signal_set_t out, ///< [in,out] First signal set.
        archi_signal_set_const_t in ///< [in] Second signal set.
);

/**
 * @brief Check if a set contains the specified signal.
 *
 * @return True if signal is in the set, false otherwise.
 */
bool
archi_signal_set_contains(
        archi_signal_set_const_t set, ///< [in] Signal set.
        int signal_index ///< [in] Signal index.
);

/**
 * @brief Add a signal to a signal set.
 *
 * If set is NULL, the function does nothing.
 */
void
archi_signal_set_add(
        archi_signal_set_t set, ///< [in] Signal set.
        int signal_index ///< [in] Signal index.
);

/**
 * @brief Remove a signal to a signal set.
 *
 * If set is NULL, the function does nothing.
 */
void
archi_signal_set_remove(
        archi_signal_set_t set, ///< [in] Signal set.
        int signal_index ///< [in] Signal index.
);

/*****************************************************************************/

/**
 * @brief Allocate and initialize a new signal flags structure.
 *
 * This function allocates an archi_signal_flags_t object
 * and initializes each per-signal flag via
 * the ARCHI_SIGNAL_FLAG_INIT macro (initializing atomics to false).
 *
 * @note The returned pointer must eventually be released via free().
 *
 * @return
 *   Pointer to a freshly allocated and initialized signal flags structure,
 *   or NULL if memory allocation fails.
 */
archi_signal_flags_t*
archi_signal_flags_alloc(void);

/**
 * @def ARCHI_SIGNAL_FLAG_IS_SET(flag)
 * @brief Atomically read the value of a signal flag.
 *
 * @param flag
 *   An atomic_bool lvalue to test.
 * @return
 *   true if the flag is set, false otherwise.
 */
#define ARCHI_SIGNAL_FLAG_IS_SET(flag)  \
    (atomic_load_explicit(&(flag), memory_order_relaxed))

/**
 * @def ARCHI_SIGNAL_FLAG_INIT(flag)
 * @brief Initialize a signal flag to false.
 *
 * Uses atomic_init to configure the atomic_bool storage.
 * Must be called before any other operations on @p flag.
 *
 * @param flag
 *   An atomic_bool lvalue to initialize.
 */
#define ARCHI_SIGNAL_FLAG_INIT(flag)    do {    \
    atomic_init(&(flag), false);                \
} while (0)

/**
 * @def ARCHI_SIGNAL_FLAG_SET(flag)
 * @brief Atomically set a signal flag (to true).
 *
 * @param flag
 *   An atomic_bool lvalue to set.
 */
#define ARCHI_SIGNAL_FLAG_SET(flag)     do {                        \
    atomic_store_explicit(&(flag), true, memory_order_relaxed);     \
} while (0)

/**
 * @def ARCHI_SIGNAL_FLAG_UNSET(flag)
 * @brief Atomically clear a signal flag (to false).
 *
 * @param flag
 *   An atomic_bool lvalue to clear.
 */
#define ARCHI_SIGNAL_FLAG_UNSET(flag)   do {                        \
    atomic_store_explicit(&(flag), false, memory_order_relaxed);    \
} while (0)

#endif // _ARCHI_SIGNAL_API_SIGNAL_FUN_H_

