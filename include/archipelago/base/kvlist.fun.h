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
#ifndef _ARCHIPELAGO_BASE_KVLIST_FUN_H_
#define _ARCHIPELAGO_BASE_KVLIST_FUN_H_

#include "archipelago/base/kvlist.typ.h"

#include <stdbool.h>

/**
 * @brief Allocate key-value list node.
 *
 * Node key is copied.
 *
 * @return Pointer to a new node, or NULL in case of failure.
 */
archi_kvlist_t*
archi_kvlist_alloc_node(
        const char *key ///< Node key.
);

/**
 * @brief Deallocate key-value list node.
 */
void
archi_kvlist_free_node(
        archi_kvlist_t *node ///< [in] Key-value list node.
);

/**
 * @brief Deallocate nodes of the whole key-value list.
 *
 * @note Circular lists are not supported.
 */
void
archi_kvlist_free(
        archi_kvlist_t *list ///< [in] Key-value list.
);

/**
 * @brief Copy key-value list.
 *
 * @note Circular lists are not supported.
 *
 * Node keys are copied.
 *
 * @return Pointer to key-value list copy head, or NULL in case of failure.
 */
archi_kvlist_t*
archi_kvlist_copy(
        const archi_kvlist_t *list, ///< [in] Original key-value list.
        archi_kvlist_t **out_tail ///< [out] Tail node of the list copy.
);

/**
 * @brief Copy key-value list, adding reference counters to values.
 *
 * @note Circular lists are not supported.
 *
 * Node keys are copied.
 * The reference counter is assigned to non-NULL pointers only.
 *
 * @return Pointer to key-value list copy head, or NULL in case of failure.
 */
archi_kvlist_rc_t*
archi_kvlist_copy_with_refcounts(
        const archi_kvlist_t *list, ///< [in] Original key-value list.
        archi_reference_count_t ref_count, ///< [in] Reference counter.
        archi_kvlist_rc_t **out_tail ///< [out] Tail node of the list copy.
);

/**
 * @brief Find a node in key-value list by key.
 *
 * @note Circular lists are not supported.
 *
 * @return True if the node was found, false if there were no such key.
 */
bool
archi_kvlist_node(
        const archi_kvlist_t *list, ///< [in] Key-value list.
        const char *key, ///< [in] Key.
        const archi_kvlist_t **out_node ///< [out] Key-value list node.
);

/**
 * @brief Get the tail node of key-value list.
 *
 * @note Circular lists are not supported.
 *
 * @return Tail node.
 */
const archi_kvlist_t*
archi_kvlist_tail(
        const archi_kvlist_t *list ///< [in] Key-value list.
);

/*****************************************************************************/

/**
 * @brief Allocate key-value list node (with reference counted pointer).
 *
 * Node key is copied.
 *
 * @return Pointer to a new node, or NULL in case of failure.
 */
archi_kvlist_rc_t*
archi_kvlist_rc_alloc_node(
        const char *key ///< Node key.
);

/**
 * @brief Deallocate key-value list node (with reference counted pointer).
 */
void
archi_kvlist_rc_free_node(
        archi_kvlist_rc_t *node ///< [in] Key-value list node.
);

/**
 * @brief Deallocate nodes of the whole key-value list (with reference counted pointers).
 *
 * @note Circular lists are not supported.
 */
void
archi_kvlist_rc_free(
        archi_kvlist_rc_t *list, ///< [in] Key-value list.
        bool decrement_ref_count ///< [in] Whether to decrement value reference counters.
);

/**
 * @brief Copy reference counted key-value list.
 *
 * @note Circular lists are not supported.
 *
 * Node keys are copied.
 *
 * @return Pointer to key-value list copy head, or NULL in case of failure.
 */
archi_kvlist_rc_t*
archi_kvlist_rc_copy(
        const archi_kvlist_rc_t *list, ///< [in] Original key-value list.
        archi_kvlist_rc_t **out_tail ///< [out] Tail node of the list copy.
);

/**
 * @brief Find a node in reference counted key-value list by key.
 *
 * @note Circular lists are not supported.
 *
 * @return True if the node was found, false if there were no such key.
 */
bool
archi_kvlist_rc_node(
        const archi_kvlist_rc_t *list, ///< [in] Key-value list.
        const char *key, ///< [in] Key.
        const archi_kvlist_rc_t **out_node ///< [out] Key-value list node.
);

/**
 * @brief Get the tail node of reference counted key-value list.
 *
 * @note Circular lists are not supported.
 *
 * @return Tail node.
 */
const archi_kvlist_rc_t*
archi_kvlist_rc_tail(
        const archi_kvlist_rc_t *list ///< [in] Key-value list.
);

#endif // _ARCHIPELAGO_BASE_KVLIST_FUN_H_

