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
 * @brief Operations on key-value lists with reference counted values.
 */

#pragma once
#ifndef _ARCHI_BASE_KRCVLIST_FUN_H_
#define _ARCHI_BASE_KRCVLIST_FUN_H_

#include "archi_base/krcvlist.typ.h"

#include <stdbool.h>


/**
 * @brief Allocate a key-value list node with reference counted value.
 *
 * Node key is copied.
 *
 * @return Pointer to a new node, or NULL in case of failure.
 */
archi_krcvlist_t*
archi_krcvlist_node_alloc(
        const char *key ///< [in] Node key.
);

/**
 * @brief Deallocate a key-value list node with reference counted value.
 *
 * Node key is freed.
 */
void
archi_krcvlist_node_free(
        archi_krcvlist_t *node ///< [in] Key-value list node.
);

/**
 * @brief Copy a key-value list with reference counted values.
 *
 * Node keys are copied.
 *
 * @note Circular lists are not supported.
 *
 * @return Head of the key-value list copy, or NULL in case of failure.
 */
archi_krcvlist_t*
archi_krcvlist_copy(
        const archi_krcvlist_t *head, ///< [in] Head of the original key-value list.
        archi_krcvlist_t **out_tail ///< [out] Tail node of the list copy.
);

/**
 * @brief Copy a key-value list, attaching a reference counter to values.
 *
 * Node keys are copied.
 * The reference counter is assigned to nodes with non-NULL value pointers only.
 *
 * @note Circular lists are not supported.
 *
 * @return Head of the key-value list copy, or NULL in case of failure.
 */
archi_krcvlist_t*
archi_krcvlist_copy_kvlist(
        const archi_kvlist_t *head, ///< [in] Head of the original key-value list.
        archi_reference_count_t ref_count, ///< [in] Assigned reference counter.
        archi_krcvlist_t **out_tail ///< [out] Tail node of the list copy.
);

/**
 * @brief Deallocate all nodes of a key-value list with reference counted values.
 *
 * @note Circular lists are not supported.
 */
void
archi_krcvlist_free(
        archi_krcvlist_t *head, ///< [in] Key-value list head.
        bool decrement_ref_counts ///< [in] Whether to decrement value reference counters.
);

#endif // _ARCHI_BASE_KRCVLIST_FUN_H_

