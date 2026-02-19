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
 * @brief Lock-free queue operations.
 */

#pragma once
#ifndef _ARCHI_THREAD_API_LFQUEUE_FUN_H_
#define _ARCHI_THREAD_API_LFQUEUE_FUN_H_

#include "archi/thread/api/handle.typ.h"
#include "archi/thread/api/lfqueue.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


/**
 * @brief Create lock-free queue.
 *
 * @return Lock-free queue handle, or NULL in case of error.
 */
archi_thread_lfqueue_t
archi_thread_lfqueue_alloc(
        archi_thread_lfqueue_alloc_params_t params, ///< [in] Lock-free queue parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Destroy lock-free queue.
 */
void
archi_thread_lfqueue_free(
        archi_thread_lfqueue_t queue ///< [in] Queue to destroy.
);

/**
 * @brief Push value to lock-free queue.
 *
 * @return True if element was pushed to queue, false if queue was full.
 */
bool
archi_thread_lfqueue_push(
        archi_thread_lfqueue_t queue, ///< [in] Queue to push value to.
        const void *value, ///< [in] Pointer to pushed value.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Pop value from lock-free queue.
 *
 * `value` may be NULL.
 *
 * @return True if element was popped from queue, false if queue was empty.
 */
bool
archi_thread_lfqueue_pop(
        archi_thread_lfqueue_t queue, ///< [in] Queue to pop value from.
        void *value, ///< [out] Memory to write popped value to.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Get queue capacity.
 *
 * @return Queue capacity.
 */
size_t
archi_thread_lfqueue_capacity(
        archi_thread_lfqueue_t queue ///< [in] Queue.
);

/**
 * @brief Get queue element size.
 *
 * @return Queue element size.
 */
size_t
archi_thread_lfqueue_elt_size(
        archi_thread_lfqueue_t queue ///< [in] Queue.
);

#endif // _ARCHI_THREAD_API_LFQUEUE_FUN_H_

