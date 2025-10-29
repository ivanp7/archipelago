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
 * @brief Memory operations.
 */

#pragma once
#ifndef _ARCHI_MEMORY_API_INTERFACE_FUN_H_
#define _ARCHI_MEMORY_API_INTERFACE_FUN_H_

#include "archi/memory/api/handle.typ.h"
#include "archi/memory/api/interface.typ.h"
#include "archipelago/base/pointer.typ.h"

/**
 * @brief Get memory interface.
 *
 * @return Pointer to the interface of the memory.
 */
archi_rcpointer_t
archi_memory_interface(
        archi_memory_t memory ///< [in] Memory.
);

/**
 * @brief Get raw memory allocation.
 *
 * @return Pointer to the raw memory allocation.
 */
archi_rcpointer_t
archi_memory_allocation(
        archi_memory_t memory ///< [in] Memory.
);

/**
 * @brief Get number of data elements in the memory.
 *
 * @return Number of data elements.
 */
size_t
archi_memory_length(
        archi_memory_t memory ///< [in] Memory.
);

/**
 * @brief Get size of a data element in the memory.
 *
 * @return Size of a data element.
 */
size_t
archi_memory_stride(
        archi_memory_t memory ///< [in] Memory.
);

/**
 * @brief Get size of the memory.
 *
 * @return Size of a the memory allocation.
 */
size_t
archi_memory_size(
        archi_memory_t memory ///< [in] Memory.
);

/**
 * @brief Get data element alignment requirement.
 *
 * @return Data element alignment requirement.
 */
size_t
archi_memory_alignment(
        archi_memory_t memory ///< [in] Memory.
);

/**
 * @brief Get memory alignment requirement.
 *
 * Memory alignment requirement is never less than data element alignment requirement.
 *
 * @return Memory alignment requirement.
 */
size_t
archi_memory_overalignment(
        archi_memory_t memory ///< [in] Memory.
);

/*****************************************************************************/

/**
 * @brief Allocate a memory object.
 *
 * If `overalignment` is 0, memory alignment requirement is equal to data element alignment requirement.
 *
 * @return New allocated memory.
 */
archi_memory_t
archi_memory_allocate(
        archi_rcpointer_t interface, ///< [in] Memory interface.
        void *alloc_data, ///< [in] Interface-specific data for allocation.

        size_t length, ///< [in] Number of data elements to allocate.
        size_t stride, ///< [in] Size of each data element.
        size_t alignment, ///< [in] Data element alignment requirement.
        size_t overalignment, ///< [in] Memory alignment requirement or 0.

        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Deallocate a memory object.
 *
 * This function is equivalent to:
 * ```c
 * archi_reference_count_decrement(archi_memory_allocation(memory).ref_count);
 * ```
 *
 * The memory object is to be considered invalid after this call.
 */
void
archi_memory_free(
        archi_memory_t memory ///< [in] Memory.
);

/*****************************************************************************/

/**
 * @brief Get the backing memory object from the memory mapping object.
 *
 * @return Pointer to the backing memory object.
 */
archi_memory_t
archi_memory_mapping_memory(
        archi_memory_mapping_t mapping ///< [in] Memory mapping.
);

/**
 * @brief Get pointer to the mapped memory region.
 *
 * @return Pointer to the mapped memory region.
 */
archi_rcpointer_t
archi_memory_mapping_pointer(
        archi_memory_mapping_t mapping ///< [in] Memory mapping.
);

/**
 * @brief Get offset of the mapped memory region.
 *
 * @return Offset of the mapped memory region.
 */
size_t
archi_memory_mapping_offset(
        archi_memory_mapping_t mapping ///< [in] Memory mapping.
);

/*****************************************************************************/

/**
 * @brief Map an area of a memory object for read/write access.
 *
 * Zero length has a special meaning: the mapped region starts from offset
 * and lasts until the memory end.
 *
 * @return Pointer to the memory mapping object.
 */
archi_memory_mapping_t
archi_memory_map(
        archi_memory_t memory, ///< [in] Memory.
        void *map_data, ///< [in] Interface-specific data for mapping.

        size_t offset, ///< [in] Offset to the mapped area in data elements.
        size_t length, ///< [in] Number of mapped data elements.

        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Unmap a previously mapped memory region.
 *
 * This function is equivalent to:
 * ```c
 * archi_reference_count_decrement(archi_memory_mapping_pointer(mapping).ref_count);
 * ```
 *
 * A memory mapping object is to be considered invalid after this call.
 */
void
archi_memory_unmap(
        archi_memory_mapping_t mapping ///< [in] Memory mapping.
);

#endif // _ARCHI_MEMORY_API_INTERFACE_FUN_H_

