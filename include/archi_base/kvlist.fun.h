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
 * @brief Operations on key-value lists.
 */

#pragma once
#ifndef _ARCHI_BASE_KVLIST_FUN_H_
#define _ARCHI_BASE_KVLIST_FUN_H_

#include "archi_base/kvlist.typ.h"
#include "archi_base/util/string.typ.h"

#include <stddef.h> // for size_t
#include <stdbool.h>


/**
 * @brief Allocate a key-value list node.
 *
 * Node key is copied.
 * if `ref_count` is true, an object of type `archi_krcvlist_t`
 * is allocated instead of `archi_kvlist_t`.
 *
 * @return Pointer to a new node, or NULL in case of failure.
 */
archi_kvlist_t*
archi_kvlist_node_alloc(
        const char *key, ///< [in] Node key.
        bool alloc_ref_count ///< [in] Whether to allocate reference counter member.
);

/**
 * @brief Deallocate a key-value list node.
 *
 * Node key is freed.
 */
void
archi_kvlist_node_free(
        archi_kvlist_t *node ///< [in] Key-value list node.
);

/*****************************************************************************/

/**
 * @brief Copy a key-value list.
 *
 * Node keys are copied.
 * The reference counter is assigned to non-NULL pointers only.
 * If `alloc_ref_count` is false, `ref_count` is ignored.
 *
 * @note Circular lists are not supported.
 *
 * @return Head of the key-value list copy, or NULL in case of failure.
 */
archi_kvlist_t*
archi_kvlist_copy(
        const archi_kvlist_t *head, ///< [in] Head of the original key-value list.
        bool alloc_ref_count, ///< [in] Whether to allocate reference counter member.
        archi_reference_count_t ref_count, ///< [in] Assigned reference counter.
        archi_kvlist_t **out_tail ///< [out] Tail node of the list copy.
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
 * @brief Deallocate all nodes of a key-value list.
 *
 * @note Circular lists are not supported.
 */
void
archi_kvlist_free(
        archi_kvlist_t *head ///< [in] Key-value list head.
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

/**
 * @brief Get the tail node of a key-value list.
 *
 * @note Circular lists are not supported.
 *
 * @return The tail node, or NULL if the list is empty.
 */
const archi_kvlist_t*
archi_kvlist_tail(
        const archi_kvlist_t *head, ///< [in] Key-value list head.
        size_t *out_length ///< [out] Length of the key-value list.
);

/**
 * @brief Get the Nth node of a key-value list.
 *
 * @return The Nth node, or NULL if the index is not less than the list length.
 */
const archi_kvlist_t*
archi_kvlist_nth_node(
        const archi_kvlist_t *head, ///< [in] Key-value list head.
        size_t index ///< [in] Node index.
);

/**
 * @brief Find the first node in a key-value list with the specified key.
 *
 * `comp_fn` may be NULL, in which case `strcmp` is used by default.
 * `key` is provided as left-hand side, while node key is provided as right-hand side.
 *
 * @note Circular lists are not supported.
 *
 * @return The first node with the specified key, or NULL if there is no such node.
 */
const archi_kvlist_t*
archi_kvlist_find(
        const archi_kvlist_t *head, ///< [in] Key-value list head.
        const char *key, ///< [in] Key to search for.
        archi_string_comp_func_t comp_fn, ///< [in] String comparison function.
        size_t *out_index ///< [out] Index of the node.
);

#endif // _ARCHI_BASE_KVLIST_FUN_H_

