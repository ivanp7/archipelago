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
 * @brief Types for threads plugin.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_THREADS_INTERFACE_TYP_H_
#define _ARCHI_PLUGIN_THREADS_INTERFACE_TYP_H_

#include "archi/plugin/threads/interface.def.h"

#include <stdbool.h>

/**
 * @brief Threaded processing function.
 */
typedef ARCHI_THREADS_TASK_FUNC((*archi_threads_task_func_t));

/**
 * @brief Threaded processing completion callback function.
 */
typedef ARCHI_THREADS_CALLBACK_FUNC((*archi_threads_callback_func_t));

/**
 * @brief Threaded processing configuration.
 */
typedef struct archi_threads_config {
    size_t num_threads; ///< Number of threads to create.
    bool busy_wait;     ///< Whether busy-waiting for a job is enabled.
} archi_threads_config_t;

/**
 * @brief Threaded processing job.
 */
typedef struct archi_threads_job {
    archi_threads_task_func_t function; ///< [in] Job task function.
    void *data;        ///< [in] Job data.
    size_t num_tasks;  ///< [in] Number of tasks in the job.
} archi_threads_job_t;

/**
 * @brief Threaded processing completion callback.
 */
typedef struct archi_threads_callback {
    archi_threads_callback_func_t function; ///< [in] Callback function.
    void *data; ///< [in] Callback data.
} archi_threads_callback_t;

#endif // _ARCHI_PLUGIN_THREADS_INTERFACE_TYP_H_

