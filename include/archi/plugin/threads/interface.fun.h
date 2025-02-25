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
 * @brief Operations with threads and concurrent processing.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_THREADS_INTERFACE_FUN_H_
#define _ARCHI_PLUGIN_THREADS_INTERFACE_FUN_H_

#include "archi/plugin/threads/interface.typ.h"
#include "archi/plugin/threads/config.typ.h"
#include "archi/app/context.typ.h"
#include "archi/util/status.typ.h"

struct archi_thread_group_context;

/**
 * @brief Initialize threaded processing context and create threads.
 *
 * This function created the specified number of threads that will be running
 * until the context is destroyed.
 * Threads wait for a job being locked or by doing a busy-wait loop.
 *
 * busy_wait parameter controls waiting behavior of threads.
 *
 * @return Threaded processing context.
 */
struct archi_thread_group_context*
archi_thread_group_start(
        archi_thread_group_config_t config, ///< [in] Threaded processing context configuration.

        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Join threads and destroy threaded processing context.
 */
void
archi_thread_group_stop(
        struct archi_thread_group_context *context ///< [in] Threaded processing context.
);

/**
 * @brief Process a job concurrently with threads.
 *
 * If value of batch_size is zero, it is replaced with ((num_tasks - 1) / num_threads) + 1.
 * With this batch size pfunc is called no more than once per thread.
 *
 * If callback function is null, the call is blocking:
 * the function does not return until all tasks are done.
 *
 * If callback function is not null, the call is non-blocking and returns immediately.
 * When all tasks are done, the callback function is called from one of the threads
 * (it is unspecified from which one).
 *
 * busy_wait value controls waiting behavior of a calling thread if callback is null.
 *
 * Sometimes the call fails (if the threads were busy). This is indicated by returning false.
 * For that reason, a loop is needed to do the job robustly:
 * while (archi_thread_group_execute(...) == 1);
 *
 * @return 0 if threads weren't busy and execution took place, 1 if threads were busy,
 * otherwise error code.
 */
archi_status_t
archi_thread_group_execute(
        struct archi_thread_group_context *context, ///< [in] Threaded processing context.

        archi_thread_group_job_t job, ///< [in] Threaded processing job.
        archi_thread_group_callback_t callback, ///< [in] Threaded processing completion callback.
        archi_thread_group_exec_config_t config ///< [in] Threaded processing configuration.
);

/**
 * @brief Get threaded processing context configuration.
 *
 * @return Threaded processing context configuration.
 */
archi_thread_group_config_t
archi_thread_group_config(
        const struct archi_thread_group_context *context ///< [in] Threaded processing context.
);

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_thread_group_context_init);   ///< Threaded processing context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_thread_group_context_final); ///< Threaded processing context finalization function.

extern
const archi_context_interface_t archi_plugin_thread_group_context_interface; ///< Threaded processing context interface functions.

/**
 * @brief Alias name of threaded processing context interface.
 */
#define ARCHI_PLUGIN_THREAD_GROUP_CONTEXT_INTERFACE_KEY "thread_group"

#endif // _ARCHI_PLUGIN_THREADS_INTERFACE_FUN_H_

