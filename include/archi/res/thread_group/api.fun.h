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
 * @brief Operations with thread groups and concurrent processing.
 */

#pragma once
#ifndef _ARCHI_RES_THREAD_GROUP_API_FUN_H_
#define _ARCHI_RES_THREAD_GROUP_API_FUN_H_

#include "archi/res/thread_group/api.typ.h"
#include "archi/util/status.typ.h"

struct archi_thread_group_context;

/**
 * @brief Start a group of threads for concurrent processing.
 *
 * This function created the specified number of threads that will be running
 * until the context is destroyed.
 * Threads wait until work is assigned to them.
 *
 * @return Thread group context.
 */
struct archi_thread_group_context*
archi_thread_group_start(
        archi_thread_group_start_params_t params, ///< [in] Thread group creation parameters.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Join threads and destroy thread group context.
 */
void
archi_thread_group_stop(
        struct archi_thread_group_context *context ///< [in] Thread group context.
);

/**
 * @brief Assign work to a thread group.
 *
 * If value of batch_size is zero, it is replaced with ((work.size - 1) / num_threads) + 1.
 * With this batch size pfunc is called no more than once per thread.
 *
 * If callback function is null, the call is blocking:
 * the function does not return until all work items are done.
 *
 * If callback function is not null, the call is non-blocking and returns immediately.
 * When all work items are done, the callback function is called from one of the threads
 * (the last one to finish it's work).
 *
 * @return 0 if threads were ready and dispatch succeeded, error code otherwise.
 */
archi_status_t
archi_thread_group_dispatch(
        struct archi_thread_group_context *context, ///< [in] Thread group context.

        archi_thread_group_work_t work, ///< [in] Concurrent work task.
        archi_thread_group_callback_t callback, ///< [in] Concurrent work completion callback.
        archi_thread_group_dispatch_params_t params ///< [in] Dispatch parameters.
);

/**
 * @brief Get thread group creation parameters.
 *
 * @return Thread group creation parameters.
 */
archi_thread_group_start_params_t
archi_thread_group_parameters(
        const struct archi_thread_group_context *context ///< [in] Thread group context.
);

#endif // _ARCHI_RES_THREAD_GROUP_API_FUN_H_

