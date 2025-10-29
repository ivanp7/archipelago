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
 * @brief Types for lock-free queue operations.
 */

#pragma once
#ifndef _ARCHI_THREAD_API_LFQUEUE_TYP_H_
#define _ARCHI_THREAD_API_LFQUEUE_TYP_H_

#include <stddef.h> // for size_t

struct archi_thread_lfqueue;

/**
 * @brief Pointer to lock-free queue.
 */
typedef struct archi_thread_lfqueue *archi_thread_lfqueue_t;

/**
 * @brief Data type tag for archi_thread_lfqueue_t.
 */
#define ARCHI_POINTER_DATA_TAG__THREAD_LFQUEUE  4

/**
 * @brief Lock-free queue allocation parameters.
 *
 * Queue capacity must be a power of two.
 * Maximum supported value of queue capacity is
 * `1 << (sizeof(uint_fastNN_t) * CHAR_BIT)`, where
 * NN is 32 if ARCHI_FEATURE_LFQUEUE32 is defined, or 16 otherwise.
 *
 * Element size can be zero, in which case no data is stored in the queue,
 * only numbers of pushes and pops are counted.
 */
typedef struct archi_thread_lfqueue_alloc_params {
    size_t capacity; ///< Queue capacity.

    struct {
        size_t size;      ///< Element size.
        size_t alignment; ///< Element alignment requirement.
    } element; ///< Queue element properties.
} archi_thread_lfqueue_alloc_params_t;

#endif // _ARCHI_THREAD_API_LFQUEUE_TYP_H_

