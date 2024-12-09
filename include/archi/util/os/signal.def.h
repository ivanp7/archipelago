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
 * @brief Macros for signal management.
 */

#pragma once
#ifndef _ARCHI_UTIL_OS_SIGNAL_DEF_H_
#define _ARCHI_UTIL_OS_SIGNAL_DEF_H_

#include "archi/util/os/signal.typ.h"
#include "archi/util/os/signal.fun.h"
#include "archi/util/flexible.def.h"

/**
 * @brief Size of signal watch set structure in bytes.
 */
#define ARCHI_SIGNAL_WATCH_SET_SIZEOF ARCHI_FLEXIBLE_SIZEOF( \
        archi_signal_watch_set_t, f_SIGRTMIN, archi_signal_number_of_rt_signals())
/**
 * @brief Size of signal flags structure in bytes.
 */
#define ARCHI_SIGNAL_FLAGS_SIZEOF ARCHI_FLEXIBLE_SIZEOF( \
        archi_signal_flags_t, f_SIGRTMIN, archi_signal_number_of_rt_signals())

/**
 * @brief Check state of a signal flag.
 */
#define ARCHI_SIGNAL_IS_FLAG_SET(flag) atomic_load_explicit(&(flag), memory_order_acquire)

/**
 * @brief Initialize a signal flag.
 */
#define ARCHI_SIGNAL_INIT_FLAG(flag) do { \
    atomic_init(&(flag), false); \
} while (0)

/**
 * @brief Set a signal flag.
 */
#define ARCHI_SIGNAL_SET_FLAG(flag) do { \
    atomic_store_explicit(&(flag), true, memory_order_release); \
} while (0)

/**
 * @brief Unset a signal flag.
 */
#define ARCHI_SIGNAL_UNSET_FLAG(flag) do { \
    atomic_store_explicit(&(flag), false, memory_order_release); \
} while (0)

/**
 * @brief Declare/define signal handler function.
 *
 * @see archi_signal_handler_func_t
 */
#define ARCHI_SIGNAL_HANDLER_FUNC(name) bool name(      \
        int signo,                                      \
        void *const restrict siginfo,                   \
        archi_signal_flags_t *const restrict signals,   \
        void *const restrict data)

#endif // _ARCHI_UTIL_OS_SIGNAL_DEF_H_

