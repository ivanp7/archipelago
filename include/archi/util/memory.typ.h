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
 * @brief Memory operation types.
 */

#pragma once
#ifndef _ARCHI_UTIL_MEMORY_TYP_H_
#define _ARCHI_UTIL_MEMORY_TYP_H_

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Parameters for archi_memory_alloc_func_t.
 */
typedef struct archi_memory_alloc_config {
    size_t num_bytes; ///< Number of bytes to allocate.
    size_t alignment; ///< Memory alignment.
} archi_memory_alloc_config_t;

/**
 * @brief Declare/define memory allocator function.
 *
 * @return Newly allocated memory or NULL.
 */
#define ARCHI_MEMORY_ALLOC_FUNC(func_name) void* func_name( \
        archi_memory_alloc_config_t *config) /* [in] Memory allocation parameters. */

/**
 * @brief Memory allocator function.
 */
typedef ARCHI_MEMORY_ALLOC_FUNC((*archi_memory_alloc_func_t));

/**
 * @brief Declare/define memory deallocator function.
 */
#define ARCHI_MEMORY_FREE_FUNC(func_name) void func_name( \
        void *restrict memory, /* [in] Memory to deallocate. */ \
        void *restrict param) /* [in] Memory deallocation parameters. */

/**
 * @brief Memory deallocator function.
 */
typedef ARCHI_MEMORY_FREE_FUNC((*archi_memory_free_func_t));

/*****************************************************************************/

/**
 * @brief Parameters for archi_memory_map_func_t.
 */
typedef struct archi_memory_map_config {
    size_t num_bytes; ///< Number of bytes to map.
    size_t offset;    ///< Offset of the mapped region.
} archi_memory_map_config_t;

/**
 * @brief Declare/define memory mapping function.
 *
 * @return True if succeed, otherwise false.
 */
#define ARCHI_MEMORY_MAP_FUNC(func_name) bool func_name( \
        void *restrict memory, /* [in] Memory to map. */ \
        archi_memory_map_config_t *restrict config) /* [in] Memory mapping parameters. */

/**
 * @brief Memory mapping function.
 */
typedef ARCHI_MEMORY_MAP_FUNC((*archi_memory_map_func_t));

/**
 * @brief Declare/define memory unmapping function.
 *
 * @return True if succeed, otherwise false.
 */
#define ARCHI_MEMORY_UNMAP_FUNC(func_name) bool func_name( \
        void *restrict memory, /* [in] Memory to unmap. */ \
        archi_memory_map_config_t *restrict config) /* [in] Memory unmapping parameters. */

/**
 * @brief Memory unmapping function.
 */
typedef ARCHI_MEMORY_UNMAP_FUNC((*archi_memory_unmap_func_t));

/*****************************************************************************/

/**
 * @brief Memory interface.
 */
typedef struct archi_memory_interface {
    archi_memory_alloc_func_t alloc_fn; ///< Memory allocation function.
    archi_memory_free_func_t free_fn;   ///< Memory deallocation function.

    archi_memory_map_func_t map_fn;     ///< Memory mapping function.
    archi_memory_unmap_func_t unmap_fn; ///< Memory unmapping function.
} archi_memory_interface_t;

#endif // _ARCHI_UTIL_MEMORY_TYP_H_

