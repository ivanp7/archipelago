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
 * @brief Operations with flag barriers.
 */

#pragma once
#ifndef _ARCHI_RES_THREAD_API_FLAG_BARRIER_FUN_H_
#define _ARCHI_RES_THREAD_API_FLAG_BARRIER_FUN_H_

#include "archipelago/base/status.typ.h"

struct archi_thread_flag_barrier;

/**
 * @brief Pointer to flag barrier.
 */
typedef struct archi_thread_flag_barrier *archi_thread_flag_barrier_t;

/**
 * @brief Allocate and initialize a flag barrier.
 *
 * @return Flag barrier.
 */
archi_thread_flag_barrier_t
archi_thread_flag_barrier_alloc(
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Destroy flag barrier.
 *
 * @warning There should not be threads waiting on the destroyed flag barrier.
 */
void
archi_thread_flag_barrier_destroy(
        archi_thread_flag_barrier_t barrier ///< [in] Flag barrier.
);

/**
 * @brief Acquire the flag, stopping threads waiting on it.
 *
 * If barrier is NULL, this function does nothing.
 *
 * @note The flag should not be acquired twice.
 *
 * @return Status code.
 */
archi_status_t
archi_thread_flag_barrier_acquire(
        archi_thread_flag_barrier_t barrier ///< [in] Flag barrier.
);

/**
 * @brief Release the flag, allowing threads waiting on it to proceed.
 *
 * If barrier is NULL, this function does nothing.
 * Upon release, all waiting threads are allowed to proceed.
 *
 * @note The flag should not be released twice.
 *
 * @return Status code.
 */
archi_status_t
archi_thread_flag_barrier_release(
        archi_thread_flag_barrier_t barrier ///< [in] Flag barrier.
);

/**
 * @brief Wait for the flag.
 *
 * If barrier is NULL, this function does nothing.
 * If the flag is not acquired, the function returns immediately.
 *
 * @return Status code.
 */
archi_status_t
archi_thread_flag_barrier_wait(
        archi_thread_flag_barrier_t barrier ///< [in] Flag barrier.
);

#endif // _ARCHI_RES_THREAD_API_FLAG_BARRIER_FUN_H_

