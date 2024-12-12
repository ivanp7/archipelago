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
 * @brief Macros for threads.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_THREADS_INTERFACE_DEF_H_
#define _ARCHI_PLUGIN_THREADS_INTERFACE_DEF_H_

#include <stddef.h>

/**
 * @brief Declare/define a threaded processing function.
 *
 * This function is called for each task in the job concurrently.
 *
 * @see archi_threads_task_func_t
 */
#define ARCHI_THREADS_TASK_FUNC(name) void name( \
        void *data, /* Job data. */ \
        size_t task_idx, /* Index of the current task. */ \
        size_t thread_idx) /* Index of the calling thread. */

/**
 * @brief Declare/define a threaded processing completion callback.
 *
 * This function is called when all tasks have been complete.
 *
 * @see archi_threads_callback_func_t
 */
#define ARCHI_THREADS_CALLBACK_FUNC(name) void name( \
        void *data, /* Callback data. */ \
        size_t num_tasks, /* Number of processed tasks. */ \
        size_t thread_idx) /* Index of the calling thread. */

/**
 * @brief Threads context configuration key -- number of threads to create.
 */
#define ARCHI_THREADS_CONFIG_KEY_NUM_THREADS "num_threads"
/**
 * @brief Threads context configuration key -- whether to enable busy-waiting for a job.
 */
#define ARCHI_THREADS_CONFIG_KEY_BUSY_WAIT "busy_wait"

#endif // _ARCHI_PLUGIN_THREADS_INTERFACE_DEF_H_

