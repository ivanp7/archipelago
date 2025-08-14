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
 * @brief Types for callback functions of thread groups.
 */

#pragma once
#ifndef _ARCHI_RES_THREAD_API_CALLBACK_TYP_H_
#define _ARCHI_RES_THREAD_API_CALLBACK_TYP_H_

#include <stddef.h> // for size_t

/**
 * @brief Declare/define a concurrent work completion callback.
 *
 * This function is called when all work items have been complete.
 */
#define ARCHI_THREAD_GROUP_CALLBACK_FUNC(name) void name( \
        void *data, /* Callback data. */ \
        size_t work_size, /* Work size. */ \
        size_t thread_idx) /* Index of the calling thread. */

/**
 * @brief Threaded processing completion callback function.
 */
typedef ARCHI_THREAD_GROUP_CALLBACK_FUNC((*archi_thread_group_callback_func_t));

/**
 * @brief Threaded processing completion callback.
 */
typedef struct archi_thread_group_callback {
    archi_thread_group_callback_func_t function; ///< [in] Callback function.
    void *data; ///< [in] Callback data.
} archi_thread_group_callback_t;

#endif // _ARCHI_RES_THREAD_API_CALLBACK_TYP_H_

