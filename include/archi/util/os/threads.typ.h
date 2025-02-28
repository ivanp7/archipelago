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
 * @brief Types for operations with threads and concurrent processing.
 */

#pragma once
#ifndef _ARCHI_UTIL_OS_THREADS_TYP_H_
#define _ARCHI_UTIL_OS_THREADS_TYP_H_

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Declare/define a threaded processing function.
 *
 * This function is called for each task in the job concurrently.
 *
 * @see archi_thread_group_task_func_t
 */
#define ARCHI_THREADS_TASK_FUNC(name) void name( \
        void *data, /* Job data. */ \
        size_t task_idx, /* Index of the current task. */ \
        size_t thread_idx) /* Index of the calling thread. */

/**
 * @brief Threaded processing function.
 */
typedef ARCHI_THREADS_TASK_FUNC((*archi_thread_group_task_func_t));

/**
 * @brief Threaded processing job.
 */
typedef struct archi_thread_group_job {
    archi_thread_group_task_func_t function; ///< [in] Job task function.
    void *data;        ///< [in] Job data.
    size_t num_tasks;  ///< [in] Number of tasks in the job.
} archi_thread_group_job_t;

/*****************************************************************************/

/**
 * @brief Declare/define a threaded processing completion callback.
 *
 * This function is called when all tasks have been complete.
 *
 * @see archi_thread_group_callback_func_t
 */
#define ARCHI_THREADS_CALLBACK_FUNC(name) void name( \
        void *data, /* Callback data. */ \
        size_t num_tasks, /* Number of processed tasks. */ \
        size_t thread_idx) /* Index of the calling thread. */

/**
 * @brief Threaded processing completion callback function.
 */
typedef ARCHI_THREADS_CALLBACK_FUNC((*archi_thread_group_callback_func_t));

/**
 * @brief Threaded processing completion callback.
 */
typedef struct archi_thread_group_callback {
    archi_thread_group_callback_func_t function; ///< [in] Callback function.
    void *data; ///< [in] Callback data.
} archi_thread_group_callback_t;

/*****************************************************************************/

/**
 * @brief Parameters for archi_thread_group_start().
 */
typedef struct archi_thread_group_config {
    size_t num_threads; ///< Number of threads to create.
    bool busy_wait;     ///< Whether busy-waiting for a job is enabled.
} archi_thread_group_config_t;

/**
 * @brief Threaded processing configuration key for the whole configuration structure.
 */
#define ARCHI_THREADS_CONFIG_KEY "config"

/**
 * @brief Threaded processing configuration key -- number of threads to create.
 */
#define ARCHI_THREADS_CONFIG_KEY_NUM_THREADS "num_threads"

/**
 * @brief Threaded processing configuration key -- whether to enable busy-waiting for a job.
 */
#define ARCHI_THREADS_CONFIG_KEY_BUSY_WAIT "busy_wait"

/*****************************************************************************/

/**
 * @brief Parameters for archi_thread_group_execute().
 */
typedef struct archi_thread_group_exec_config {
    size_t batch_size; ///< Number of tasks done by a thread at a time.
    bool busy_wait;    ///< Whether busy-waiting for job completion is enabled.
} archi_thread_group_exec_config_t;

#endif // _ARCHI_UTIL_OS_THREADS_TYP_H_

