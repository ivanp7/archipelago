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
 * @brief Lock-free queue operations.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_THREADS_QUEUE_FUN_H_
#define _ARCHI_PLUGIN_THREADS_QUEUE_FUN_H_

#include "archi/util/os/queue.typ.h"

struct archi_queue;

/**
 * @brief Create lock-free queue.
 *
 * Maximum queue capacity is (1 << capacity_log2) elements.
 * Maximum supported value of capacity_log2 is 16 (or 32,
 * if larger queues were enabled at build time).
 *
 * @return Lock-free queue.
 */
struct archi_queue*
archi_queue_alloc(
        archi_queue_config_t config ///< [in] Lock-free queue configuration.
);

/**
 * @brief Destroy lock-free queue.
 */
void
archi_queue_free(
        struct archi_queue *queue ///< [in] Queue to destroy.
);

/**
 * @brief Push value to lock-free queue.
 *
 * @return True if element was pushed to queue, false if queue is full.
 */
bool
archi_queue_push(
        struct archi_queue *restrict queue, ///< [in] Queue to push value to.
        const void *restrict value ///< [in] Pointer to pushed value.
);

/**
 * @brief Pop value from lock-free queue.
 *
 * @return True if element was popped from queue, false if queue is empty.
 */
bool
archi_queue_pop(
        struct archi_queue *restrict queue, ///< [in] Queue to pop value from.
        void *restrict value ///< [out] Memory to write popped value to.
);

/**
 * @brief Get queue capacity.
 *
 * @return Queue capacity.
 */
size_t
archi_queue_capacity(
        struct archi_queue *queue ///< [in] Queue.
);

/**
 * @brief Get queue element size.
 *
 * @return Queue element size.
 */
size_t
archi_queue_element_size(
        struct archi_queue *queue ///< [in] Queue.
);

#endif // _ARCHI_PLUGIN_THREADS_QUEUE_FUN_H_

