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
 * @brief Custom memory management interface.
 */

#pragma once
#ifndef _ARCHI_MEMORY_API_INTERFACE_TYP_H_
#define _ARCHI_MEMORY_API_INTERFACE_TYP_H_

#include "archipelago/base/pointer.typ.h"
#include "archipelago/base/error.typ.h"

#include <stddef.h> // for size_t
#include <stdbool.h>

/**
 * @struct archi_memory_alloc_info_t
 * @brief Memory allocation description.
 *
 * Groups together pointers to the allocated memory and its metadata.
 *
 * @note
 *   - The `allocation.ptr` pointer is mandatory.
 *   - The `metadata` pointer is optional.
 *   - If `allocation.tag` is 0, the pointer is considered transparent.
 */
typedef struct archi_memory_alloc_info {
    struct {
        void *ptr; ///< Pointer/handle of allocated memory.
        archi_pointer_attr_t tag; ///< Allocation handle type tag or 0.
        bool writable; ///< Whether allocation is writable.
    } allocation;
    void *metadata; ///< Metadata for allocated memory.
} archi_memory_alloc_info_t;

/**
 * @struct archi_memory_map_info_t
 * @brief Memory mapping description.
 *
 * Groups together pointers to the mapped memory and its metadata.
 *
 * @note
 *   - The `mapping.ptr` pointer is mandatory.
 *   - The `metadata` pointer is optional.
 */
typedef struct archi_memory_map_info {
    struct {
        void *ptr; ///< Pointer to mapped memory.
        bool writable; ///< Whether mapping is writable.
    } mapping;
    void *metadata; ///< Metadata for mapped memory.
} archi_memory_map_info_t;

/**
 * @brief Declare/define a memory allocator function.
 *
 * This function is intended for allocating the memory.
 *
 * Contract:
 * (1) return non-NULL allocation pointer and zero error code on success;
 * (2) return NULL allocation pointer and non-zero error code on failure.
 *
 * @return Memory allocation description.
 */
#define ARCHI_MEMORY_ALLOC_FUNC(func_name)  archi_memory_alloc_info_t func_name(    \
        size_t num_bytes, /* [in] Number of bytes to allocate. */                   \
        size_t alignment, /* [in] Memory alignment requirement. */                  \
        void *alloc_data, /* [in] Interface-specific data for allocation. */        \
        ARCHI_ERROR_PARAMETER_DECL) /* [out] Error. */

/**
 * @brief Memory allocator function type.
 */
typedef ARCHI_MEMORY_ALLOC_FUNC((*archi_memory_alloc_func_t));

/**
 * @brief Declare/define a memory deallocator function.
 *
 * This function is intended for releasing the allocated memory.
 */
#define ARCHI_MEMORY_FREE_FUNC(func_name)   void func_name( \
        archi_memory_alloc_info_t alloc_info) /* [in] Memory allocation description. */

/**
 * @brief Memory deallocator function type.
 */
typedef ARCHI_MEMORY_FREE_FUNC((*archi_memory_free_func_t));

/**
 * @brief Declare/define a memory mapping function.
 *
 * This function is intended for mapping a memory region for access.
 *
 * Contract:
 * (1) return non-NULL mapping pointer and zero error code on success;
 * (2) return NULL mapping pointer and non-zero error code on failure.
 *
 * @return Memory mapping description.
 */
#define ARCHI_MEMORY_MAP_FUNC(func_name)    archi_memory_map_info_t func_name(  \
        archi_memory_alloc_info_t alloc_info, /* [in] Allocation. */            \
        size_t offset, /* [in] Offset of mapped region. */                      \
        size_t num_bytes, /* [in] Size of mapped region. */                     \
        void *map_data, /* [in] Interface-specific data for mapping. */         \
        ARCHI_ERROR_PARAMETER_DECL) /* [out] Error. */

/**
 * @brief Memory mapping function type.
 */
typedef ARCHI_MEMORY_MAP_FUNC((*archi_memory_map_func_t));

/**
 * @brief Declare/define a memory unmapping function.
 *
 * This function is intended for unmapping a mapped memory region.
 */
#define ARCHI_MEMORY_UNMAP_FUNC(func_name)  void func_name(                             \
        archi_memory_alloc_info_t alloc_info, /* [in] Memory allocation description. */ \
        archi_memory_map_info_t map_info) /* [in] Memory mapping description. */

/**
 * @brief Memory unmapping function type.
 */
typedef ARCHI_MEMORY_UNMAP_FUNC((*archi_memory_unmap_func_t));

/*****************************************************************************/

/**
 * @struct archi_memory_interface_t
 * @brief Memory interface functions.
 */
typedef struct archi_memory_interface {
    archi_memory_alloc_func_t alloc_fn; ///< Memory allocation function.
    archi_memory_free_func_t free_fn;   ///< Memory deallocation function.

    archi_memory_map_func_t map_fn;     ///< Memory mapping function.
    archi_memory_unmap_func_t unmap_fn; ///< Memory unmapping function.
} archi_memory_interface_t;

#endif // _ARCHI_MEMORY_API_INTERFACE_TYP_H_

