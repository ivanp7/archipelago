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
 * @brief Types for operations on hashmaps.
 */

#pragma once
#ifndef _ARCHI_HASHMAP_API_HASHMAP_TYP_H_
#define _ARCHI_HASHMAP_API_HASHMAP_TYP_H_

#include "archi_base/pointer.typ.h"

#include <stddef.h>
#include <stdbool.h>


/**
 * @brief Hashmap allocation parameters.
 *
 * If capacity is 0, default capacity is used instead.
 *
 * When capacity is 1, the hashmap behavior is equivalent to a simple linked list.
 */
typedef struct archi_hashmap_alloc_params {
    size_t capacity; ///< Hashmap array capacity.
} archi_hashmap_alloc_params_t;

/**
 * @brief Default hashmap capacity.
 */
#define ARCHI_HASHMAP_DEFAULT_CAPACITY  1024

/**
 * @brief Signature of a hashmap hash function.
 *
 * @return Hash of a key.
 */
#define ARCHI_HASHMAP_HASH_FUNC(func_name)  size_t func_name(   \
        const char *key) /* Key. */

/**
 * @brief Hashmap hash function type.
 */
typedef ARCHI_HASHMAP_HASH_FUNC((*archi_hashmap_hash_func_t));

/*****************************************************************************/

/**
 * @brief Signature of a hashmap key-value function.
 *
 * @return True if the operation is confirmed, false if it is canceled.
 */
#define ARCHI_HASHMAP_KV_FUNC(func_name)    bool func_name( \
        const char *key, /* Key. */                         \
        archi_rcpointer_t value, /* Value. */               \
        void *data) /* Function data. */

/**
 * @brief Hashmap key-value function type.
 */
typedef ARCHI_HASHMAP_KV_FUNC((*archi_hashmap_kv_func_t));

/**
 * @brief Hashmap setter parameters.
 *
 * Both insertion and updating cannot be disallowed at the same time.
 */
typedef struct archi_hashmap_set_params {
    archi_hashmap_kv_func_t set_fn; ///< Key-value function called for the old value.
    void *set_fn_data; ///< Key-value function data.

    bool insertion_allowed; ///< Whether insertion of the new key is allowed.
    bool update_allowed;    ///< Whether updating value of the existing key is allowed.
} archi_hashmap_set_params_t;

/**
 * @brief Hashmap unsetter parameters.
 */
typedef struct archi_hashmap_unset_params {
    archi_hashmap_kv_func_t unset_fn; ///< Key-value function called for the old value.
    void *unset_fn_data; ///< Key-value function data.
} archi_hashmap_unset_params_t;

/*****************************************************************************/

/**
 * @brief Hashmap traversal key-value action type.
 */
typedef enum archi_hashmap_trav_action_type {
    ARCHI_HASHMAP_TRAV_KEEP = 0, ///< Don't change the value.
    ARCHI_HASHMAP_TRAV_SET,      ///< Set the new value.
    ARCHI_HASHMAP_TRAV_UNSET,    ///< Unset the value and remove the key.
} archi_hashmap_trav_action_type_t;

/**
 * @brief Hashmap traversal key-value action.
 */
typedef struct archi_hashmap_trav_action {
    bool interrupt; ///< Whether to interrupt the traversal after the action.
    archi_hashmap_trav_action_type_t type; ///< Action type.
    archi_rcpointer_t new_value; ///< New value.
} archi_hashmap_trav_action_t;

/**
 * @brief Signature of a hashmap traversal key-value action function.
 *
 * @return Traversal action.
 */
#define ARCHI_HASHMAP_TRAV_KV_FUNC(func_name)   archi_hashmap_trav_action_t func_name(  \
        const char *key, /* Key. */                                                     \
        archi_rcpointer_t value, /* Value. */                                           \
        size_t index, /* Number of the current key-value pair. */                       \
        void *data) /* Function data. */

/**
 * @brief Hashmap traversal key-value function type.
 */
typedef ARCHI_HASHMAP_TRAV_KV_FUNC((*archi_hashmap_trav_kv_func_t));

#endif // _ARCHI_HASHMAP_API_HASHMAP_TYP_H_

