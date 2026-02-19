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
 * @brief Operations on hashmaps.
 */

#pragma once
#ifndef _ARCHI_HASHMAP_API_HASHMAP_FUN_H_
#define _ARCHI_HASHMAP_API_HASHMAP_FUN_H_

#include "archi/hashmap/api/handle.typ.h"
#include "archi/hashmap/api/hashmap.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Allocate a hashmap.
 *
 * If `hash_fn` is NULL, `archi_string_hash()` is used by default.
 *
 * @return Hashmap handle, or NULL in case of error.
 */
archi_hashmap_t
archi_hashmap_alloc(
        archi_hashmap_hash_func_t hash_fn, ///< [in] Hashmap hash function.
        archi_hashmap_alloc_params_t params, ///< [in] Hashmap allocation parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Deallocate a hashmap.
 */
void
archi_hashmap_free(
        archi_hashmap_t hashmap ///< [in] Hashmap.
);

/**
 * @brief Get a value for the specified key in the hashmap.
 *
 * Parameter `value` is written to only when such key exists in the hashmap.
 *
 * @return True if the value with such key was found, otherwise false.
 */
bool
archi_hashmap_get(
        archi_hashmap_t hashmap, ///< [in] Hashmap.
        const char *key, ///< [in] Key.
        archi_rcpointer_t *value, ///< [out] The value associated with the key.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Set a value for the specified key in the hashmap.
 *
 * @return True if the value was inserted, otherwise false.
 */
bool
archi_hashmap_set(
        archi_hashmap_t hashmap, ///< [in] Hashmap.
        const char *key, ///< [in] Key.
        archi_rcpointer_t value, ///< [in] Value.
        archi_hashmap_set_params_t params, ///< [in] Additional parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Unset a value for the specified key in the hashmap.
 *
 * @return True if the value was removed, otherwise false.
 */
bool
archi_hashmap_unset(
        archi_hashmap_t hashmap, ///< [in] Hashmap.
        const char *key, ///< [in] Key.
        archi_hashmap_unset_params_t params, ///< [in] Additional parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Traverse the hashmap, callin a function for all key-value pairs.
 *
 * @return True if all key-value pairs have been traversed, otherwise false.
 */
bool
archi_hashmap_traverse(
        archi_hashmap_t hashmap, ///< [in] Hashmap.
        bool first_to_last, ///< [in] True for insertion order (first-to-last), false for reverse insertion order (last-to-first).
        archi_hashmap_trav_kv_func_t trav_fn, ///< [in] Traversal function.
        void *trav_fn_data, ///< [in] Traversal function data.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Get number of elements in the hashmap.
 *
 * @return Number of elements in the hashmap.
 */
size_t
archi_hashmap_num_elements(
        archi_hashmap_t hashmap ///< [in] Hashmap.
);

/**
 * @brief Get capacity of the hashmap.
 *
 * @return Hashmap capacity.
 */
size_t
archi_hashmap_capacity(
        archi_hashmap_t hashmap ///< [in] Hashmap.
);

/*****************************************************************************/

/**
 * @brief Hashmap traversal key-value function: unset everything.
 */
ARCHI_HASHMAP_TRAV_KV_FUNC(archi_hashmap_trav_kv__unset_all);

#endif // _ARCHI_HASHMAP_API_HASHMAP_FUN_H_

