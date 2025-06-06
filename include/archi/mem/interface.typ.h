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
 * @brief Custom memory management interface.
 */

#pragma once
#ifndef _ARCHI_MEM_INTERFACE_TYP_H_
#define _ARCHI_MEM_INTERFACE_TYP_H_

#include "archi/util/status.typ.h"

#include <stddef.h> // for size_t
#include <stdbool.h>

/**
 * @def ARCHI_MEMORY_ALLOC_FUNC(func_name)
 * @brief Declare or define a memory allocator function.
 *
 * Expands to the signature required for any memory allocator:
 *
 * @param[in] num_bytes
 *   Number of bytes to allocate. Must be greater than zero.
 *
 * @param[in] alignment
 *   Memory allocation alignment requirement. Must be a power of two,
 *   or zero (request of the maximum alignment).
 *
 * @param[in] alloc_data
 *   Interface-specific data needed for allocation.
 *
 * @param[out] code
 *   Status code of the allocation:
 *     - negative on failure;
 *     - zero on success (or failure if NULL is returned);
 *     - positive on an interface-specific condition (either success or failure).
 *
 * @return
 *     Pointer to the object representing allocated memory on success,
 *     or NULL on failure.
 *
 * The allocator must satisfy the following contract:
 *  1. On success: return a non-NULL pointer and set *code >= 0.
 *  2. On failure: return NULL and set *code <= 0.
 *
 * The contract is violated when *code is set to a negative (error) value,
 * but a non-NULL pointer is returned.
 *
 * @see archi_memory_alloc_func_t
 */
#define ARCHI_MEMORY_ALLOC_FUNC(func_name)  void* func_name( \
        size_t num_bytes, \
        size_t alignment, \
        void *alloc_data, \
        archi_status_t *code)

/**
 * @brief Function pointer type for a memory allocator.
 *
 * Points to a function with the signature generated by ARCHI_MEMORY_ALLOC_FUNC().
 *
 * @see ARCHI_MEMORY_ALLOC_FUNC
 */
typedef ARCHI_MEMORY_ALLOC_FUNC((*archi_memory_alloc_func_t));

/**
 * @brief Declare or define a memory deallocator function.
 *
 * Expands to the signature required for any memory deallocator:
 *
 * @param[in] allocation
 *   Pointer to the object representing memory previously allocated
 *   by a matching allocation function.
 *   May be NULL, in which case no operation is to be performed.
 *
 * @see archi_memory_free_func_t
 */
#define ARCHI_MEMORY_FREE_FUNC(func_name)   void func_name( \
        void *allocation)

/**
 * @brief Function pointer type for a memory deallocator.
 *
 * Points to a function with the signature generated by ARCHI_MEMORY_FREE_FUNC().
 *
 * @see ARCHI_MEMORY_FREE_FUNC
 */
typedef ARCHI_MEMORY_FREE_FUNC((*archi_memory_free_func_t));

/**
 * @brief Declare or define a memory mapping function.
 *
 * Expands to the signature required for any memory mapper:
 *
 * @param[in] allocation
 *   Pointer to the object representing memory previously allocated
 *   by a matching allocation function. Must not be NULL.
 *
 * @param[in] offset
 *   Offset to the mapped memory area in bytes. Must not be out-of-range.
 *
 * @param[in] num_bytes
 *   Size of the mapped memory area in bytes. Must not exceed the allocation limits.
 *
 * @param[in] for_writing
 *   Whether the memory area is mapped for writing.
 *
 * @param[in] map_data
 *   Interface-specific data needed for mapping.
 *
 * @param[out] code
 *   Status code of the operation:
 *     - negative on failure,
 *     - zero on success,
 *     - positive on a interface-specific condition (either success or failure).
 *
 * @return
 *   Pointer to the mapped memory area on success.
 *   NULL on failure (with corresponding status code set in @p code).
 *
 * @see archi_memory_map_func_t
 */
#define ARCHI_MEMORY_MAP_FUNC(func_name)    void* func_name( \
        void *allocation, \
        size_t offset, \
        size_t num_bytes, \
        bool for_writing, \
        void *map_data, \
        archi_status_t *code)

/**
 * @brief Function pointer type for a memory‑mapping routine.
 *
 * Points to a function with the signature generated by ARCHI_MEMORY_MAP_FUNC().
 *
 * @see ARCHI_MEMORY_MAP_FUNC
 */
typedef ARCHI_MEMORY_MAP_FUNC((*archi_memory_map_func_t));

/**
 * @brief Declare or define a memory unmapping function.
 *
 * Expands to the signature required for any memory unmapper:
 *
 * @param[in] allocation
 *   Pointer to the object representing memory previously allocated
 *   by a matching allocation function. Must not be NULL.
 *
 * @param[in] mapping
 *   Pointer to the memory area previously mapped by a matching mapping function.
 *   May be NULL, in which case no operation is to be performed.
 *
 * @see archi_memory_unmap_func_t
 */
#define ARCHI_MEMORY_UNMAP_FUNC(func_name)  void func_name( \
        void *allocation, \
        void *mapping)

/**
 * @brief Function‑pointer type for a memory unmapping routine.
 *
 * Points to a function with the signature generated by ARCHI_MEMORY_UNMAP_FUNC().
 *
 * @see ARCHI_MEMORY_UNMAP_FUNC
 */
typedef ARCHI_MEMORY_UNMAP_FUNC((*archi_memory_unmap_func_t));

/*****************************************************************************/

/**
 * @struct archi_memory_interface_t
 * @brief Custom memory‐management interface.
 *
 * Groups together routines for dynamic memory
 * allocation, deallocation, mapping, and unmapping.
 *
 * @note
 *   - The allocation function (alloc_fn) is mandatory.
 *   - The deallocation (free_fn), mapping (map_fn) and unmapping
 *     (unmap_fn) functions are optional. If any of these pointers
 *     is NULL, the corresponding operation is treated as a no‐op
 *     (i.e., does nothing and returns success).
 *
 * @note
 *   All four function pointers must conform to the respective signatures generated by:
 *     - ARCHI_MEMORY_ALLOC_FUNC
 *     - ARCHI_MEMORY_FREE_FUNC
 *     - ARCHI_MEMORY_MAP_FUNC
 *     - ARCHI_MEMORY_UNMAP_FUNC
 */
typedef struct archi_memory_interface {
    archi_memory_alloc_func_t alloc_fn; ///< Memory allocation function.
    archi_memory_free_func_t free_fn;   ///< Memory deallocation function.

    archi_memory_map_func_t map_fn;     ///< Memory mapping function.
    archi_memory_unmap_func_t unmap_fn; ///< Memory unmapping function.
} archi_memory_interface_t;

#endif // _ARCHI_MEM_INTERFACE_TYP_H_

