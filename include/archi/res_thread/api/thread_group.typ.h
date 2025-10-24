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
 * @brief Types for operations with thread groups and concurrent processing.
 */

#pragma once
#ifndef _ARCHI_RES_THREAD_API_THREAD_GROUP_TYP_H_
#define _ARCHI_RES_THREAD_API_THREAD_GROUP_TYP_H_

#include <stddef.h> // for size_t

struct archi_thread_group_context;

/**
 * @brief Pointer to thread group context.
 */
typedef struct archi_thread_group_context *archi_thread_group_context_t;

/**
 * @brief Parameters for archi_thread_group_start().
 */
typedef struct archi_thread_group_start_params {
    size_t num_threads; ///< Number of threads to create.
} archi_thread_group_start_params_t;

/**
 * @brief Additional parameters for archi_thread_group_dispatch().
 */
typedef struct archi_thread_group_dispatch_params {
    size_t batch_size; ///< Number of work items done by a thread at once.
} archi_thread_group_dispatch_params_t;

#endif // _ARCHI_RES_THREAD_API_THREAD_GROUP_TYP_H_

