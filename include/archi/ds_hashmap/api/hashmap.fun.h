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
 * @brief Operations on hashmaps.
 */

#pragma once
#ifndef _ARCHI_DS_HASHMAP_API_HASHMAP_FUN_H_
#define _ARCHI_DS_HASHMAP_API_HASHMAP_FUN_H_

#include "archi/ds_hashmap/api/hashmap.typ.h"
#include "archipelago/base/status.typ.h"

struct archi_hashmap;

/**
 * @brief Pointer to hashmap.
 */
typedef struct archi_hashmap *archi_hashmap_t;

/**
 * @brief Compute hash of a string.
 *
 * This function implements the djb2 algorithm.
 *
 * @return Hash value.
 */
size_t
archi_hash(
        const char *string ///< [in] A string.
);

/**
 * @brief Allocate a hashmap.
 *
 * @return Hashmap.
 */
archi_hashmap_t
archi_hashmap_alloc(
        archi_hashmap_alloc_params_t params, ///< [in] Hashmap allocation parameters.
        archi_status_t *code ///< [out] Status code.
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
 * Status code:
 * <0 - error;
 * 0 - success;
 * 1 - the key does not exist.
 *
 * @return The value associated with the key.
 */
archi_pointer_t
archi_hashmap_get(
        archi_hashmap_t hashmap, ///< [in] Hashmap.

        const char *key, ///< [in] Key.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Set a value for the specified key in the hashmap.
 *
 * @return Status code:
 * <0 - error;
 * 0 - success;
 * 1 - the key doesn't exist and key insertion is disallowed;
 * 2 - the key exists and value updating is disallowed;
 * 3 - the key exists and the key-value function returned false.
 */
archi_status_t
archi_hashmap_set(
        archi_hashmap_t hashmap, ///< [in] Hashmap.

        const char *key, ///< [in] Key.
        archi_pointer_t value, ///< [in] Value.
        archi_hashmap_set_params_t params ///< [in] Additional parameters.
);

/**
 * @brief Unset a value for the specified key in the hashmap.
 *
 * @return Status code:
 * <0 - error;
 * 0 - success;
 * 1 - the key does not exist;
 * (2 is not used);
 * 3 - the key exists and the key-value function returned false.
 */
archi_status_t
archi_hashmap_unset(
        archi_hashmap_t hashmap, ///< [in] Hashmap.

        const char *key, ///< [in] Key.
        archi_hashmap_unset_params_t params ///< [in] Additional parameters.
);

/**
 * @brief Traverse the hashmap, callin a function for all key-value pairs.
 *
 * @return Status code:
 * <0 - error;
 * 0 - success;
 * 1 - hashmap traversal has been interrupted.
 */
archi_status_t
archi_hashmap_traverse(
        archi_hashmap_t hashmap, ///< [in] Hashmap.

        bool first_to_last, ///< [in] True for insertion order (first-to-last), false for reverse insertion order (last-to-first).
        archi_hashmap_trav_kv_func_t trav_fn, ///< [in] Traversal function.
        void *trav_fn_data ///< [in] Traversal function data.
);

/**
 * @brief Get number of elements in the hashmap.
 *
 * @return Number of elements in the hashmap.
 */
size_t
archi_hashmap_size(
        archi_hashmap_t hashmap ///< [in] Hashmap.
);

#endif // _ARCHI_DS_HASHMAP_API_HASHMAP_FUN_H_

