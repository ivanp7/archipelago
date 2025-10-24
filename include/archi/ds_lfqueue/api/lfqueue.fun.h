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
#ifndef _ARCHI_DS_LFQUEUE_API_LFQUEUE_FUN_H_
#define _ARCHI_DS_LFQUEUE_API_LFQUEUE_FUN_H_

#include "archi/ds_lfqueue/api/lfqueue.typ.h"
#include "archipelago/base/status.typ.h"

#include <stdbool.h>

/**
 * @brief Create lock-free queue.
 *
 * Maximum queue capacity is (1 << capacity_log2) elements.
 * Maximum supported value of capacity_log2 is 16 (or 32,
 * if larger queues were enabled at build time).
 *
 * @return Lock-free queue.
 */
archi_lfqueue_t
archi_lfqueue_alloc(
        archi_lfqueue_alloc_params_t params, ///< [in] Lock-free queue parameters.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Destroy lock-free queue.
 */
void
archi_lfqueue_free(
        archi_lfqueue_t queue ///< [in] Queue to destroy.
);

/**
 * @brief Push value to lock-free queue.
 *
 * @return True if element was pushed to queue, false if queue is full.
 */
bool
archi_lfqueue_push(
        archi_lfqueue_t restrict queue, ///< [in] Queue to push value to.
        const void *restrict value ///< [in] Pointer to pushed value.
);

/**
 * @brief Pop value from lock-free queue.
 *
 * @return True if element was popped from queue, false if queue is empty.
 */
bool
archi_lfqueue_pop(
        archi_lfqueue_t restrict queue, ///< [in] Queue to pop value from.
        void *restrict value ///< [out] Memory to write popped value to.
);

/**
 * @brief Get queue capacity.
 *
 * @return Queue capacity.
 */
size_t
archi_lfqueue_capacity(
        archi_lfqueue_t queue ///< [in] Queue.
);

/**
 * @brief Get queue element size.
 *
 * @return Queue element size.
 */
size_t
archi_lfqueue_element_size(
        archi_lfqueue_t queue ///< [in] Queue.
);

#endif // _ARCHI_DS_LFQUEUE_API_LFQUEUE_FUN_H_

