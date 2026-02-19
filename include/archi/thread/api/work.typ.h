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
 * @brief Types for describing work for thread groups.
 */

#pragma once
#ifndef _ARCHI_THREAD_API_WORK_TYP_H_
#define _ARCHI_THREAD_API_WORK_TYP_H_

#include <stddef.h> // for size_t


/**
 * @brief Declare/define a concurrent work function.
 *
 * This function is called for each work item concurrently.
 */
#define ARCHI_THREAD_GROUP_WORK_FUNC(func_name)     void func_name(         \
        void *data, /* [in] Work data. */                                   \
        size_t work_item_idx, /* [in] Index of the current work item. */    \
        size_t thread_idx) /* [in] Index of the calling thread. */

/**
 * @brief Concurrent work function.
 */
typedef ARCHI_THREAD_GROUP_WORK_FUNC((*archi_thread_group_work_func_t));

/**
 * @brief Concurrent work task.
 */
typedef struct archi_thread_group_work {
    archi_thread_group_work_func_t function; ///< Work function.
    void *data; ///< Work data.
} archi_thread_group_work_t;

#endif // _ARCHI_THREAD_API_WORK_TYP_H_

