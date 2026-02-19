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
 * @brief Operations with thread groups and concurrent processing.
 */

#pragma once
#ifndef _ARCHI_THREAD_API_THREAD_GROUP_FUN_H_
#define _ARCHI_THREAD_API_THREAD_GROUP_FUN_H_

#include "archi/thread/api/handle.typ.h"
#include "archi/thread/api/thread_group.typ.h"
#include "archi/thread/api/work.typ.h"
#include "archi/thread/api/callback.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


struct timespec;

/**
 * @brief Create a group of threads for concurrent processing.
 *
 * This function creates the specified number of threads that will be running
 * until the context is destroyed.
 * Threads wait until work is assigned to them.
 *
 * @return Thread group.
 */
archi_thread_group_t
archi_thread_group_create(
        archi_thread_group_start_params_t params, ///< [in] Thread group creation parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Join threads and destroy thread group context.
 */
void
archi_thread_group_destroy(
        archi_thread_group_t thread_group ///< [in] Thread group.
);

/**
 * @brief Assign work to a thread group.
 *
 * If value of batch_size is zero, it is replaced with ((work.size - 1) / num_threads) + 1.
 * With this batch size pfunc is called no more than once per thread.
 *
 * When all work items are done, the callback function is called from one of the threads
 * (the last one to finish).
 *
 * @return True if work has been assigned, false otherwise.
 */
bool
archi_thread_group_dispatch(
        archi_thread_group_t thread_group, ///< [in] Thread group.

        archi_thread_group_work_t work, ///< [in] Concurrent work task.
        archi_thread_group_callback_t callback, ///< [in] Concurrent work completion callback.

        archi_thread_group_dispatch_params_t params, ///< [in] Dispatch parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Wait until work task completion.
 *
 * If thread group is not busy, the function returns immediately.
 */
void
archi_thread_group_wait(
        archi_thread_group_t thread_group ///< [in] Thread group.
);

/**
 * @brief Wait until work task completion or timeout.
 *
 * If `time_point` is NULL, the wait time is unbounded as by using `archi_thread_group_wait()`.
 *
 * If thread group is not busy, the function returns immediately.
 */
void
archi_thread_group_wait_until(
        archi_thread_group_t thread_group, ///< [in] Thread group.
        const struct timespec *time_point  ///< [in] TIME_UTC based time point of timeout.
);

/**
 * @brief Get number of threads in a group.
 *
 * @return Number of threads.
 */
size_t
archi_thread_group_num_threads(
        archi_thread_group_t thread_group ///< [in] Thread group.
);

#endif // _ARCHI_THREAD_API_THREAD_GROUP_FUN_H_

