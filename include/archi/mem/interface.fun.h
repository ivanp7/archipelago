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
 * @brief Memory operations.
 */

#pragma once
#ifndef _ARCHI_MEM_INTERFACE_FUN_H_
#define _ARCHI_MEM_INTERFACE_FUN_H_

#include "archi/mem/interface.typ.h"
#include "archi/util/size.typ.h"
#include "archi/util/pointer.typ.h"

struct archi_memory;

/**
 * @brief Extract memory interface.
 *
 * @return Pointer the interface of the memory.
 */
archi_pointer_t
archi_memory_interface(
        struct archi_memory *memory ///< [in] Memory.
);

/**
 * @brief Extract memory layout.
 *
 * @return Memory layout.
 */
archi_array_layout_t
archi_memory_layout(
        struct archi_memory *memory ///< [in] Context.
);

/**
 * @brief Extract mapped memory area.
 *
 * @return Pointer wrapper to the mapped memory area.
 */
archi_pointer_t
archi_memory_data(
        struct archi_memory *memory ///< [in] Context.
);

/*****************************************************************************/

/**
 * @brief Allocate a memory object.
 *
 * @return New allocated memory.
 */
struct archi_memory*
archi_memory_allocate(
        archi_pointer_t interface, ///< [in] Memory interface.
        void *alloc_data, ///< [in] Interface-specific data for allocation.

        archi_array_layout_t layout, ///< [in] Memory array layout.

        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Deallocate a memory object.
 *
 * This is done forcibly, without considering the reference count state.
 * If an area of the memory is mapped, it is unmapped.
 */
void
archi_memory_free(
        struct archi_memory *memory ///< [in] Memory.
);

/**
 * @brief Map an area of a memory object for read/write access.
 *
 * @return Pointer wrapper to the mapped memory area.
 */
archi_pointer_t
archi_memory_map(
        struct archi_memory *memory, ///< [in] Memory.
        void *map_data, ///< [in] Interface-specific data for mapping.

        size_t offset, ///< [in] Offset to the mapped area in data elements.
        size_t num_of, ///< [in] Number of mapped data elements.
        bool writeable, ///< [in] Whether the memory is mapped for writing.

        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Unmap a previously mapped memory region.
 */
void
archi_memory_unmap(
        struct archi_memory *memory ///< [in] Memory.
);

/*****************************************************************************/

/**
 * @brief Copy a chunk of data between two memory regions.
 *
 * This routine performs the following steps:
 *   1. If size == 0, return success immediately.
 *   2. Validate that both source and destination memory are non‐NULL, have NULL mapping,
 *   have matching element sizes and alignment, and the copied regions are valid.
 *   3. Map the source region for access via archi_memory_map().
 *   4. Map the destination region for access via archi_memory_map().
 *      If mapping the destination fails, the source mapping is undone.
 *   5. memcpy() num_of data elements from source->mapping.ptr to destination->mapping.ptr.
 *   6. Unmap the destination, then unmap the source via archi_memory_unmap().
 *
 * @note The source and destination memory must not be the same.
 *
 * @param [in,out] memory_dest
 *   Wrapper for the destination memory region. A temporary mapping is
 *   created and torn down during this call; on return mapping_ptr == NULL.
 *
 * @param [in,out] memory_src
 *   Wrapper for the source memory region. A temporary mapping is created
 *   and torn down during this call; on return mapping_ptr == NULL.
 *
 * @param [in] num_of
 *   Number of data elements to copy. May be 0, in which case nothing is done.
 *
 * @return Status code.
 *   @retval 0                     Copy succeeded (or size == 0).
 *   @retval ARCHI_STATUS_EMISUSE  Invalid arguments or size exceeds mapping bounds.
 *   @retval <other>               Error code forwarded from
 *                                 archi_memory_map() or archi_memory_unmap().
 */
archi_status_t
archi_memory_map_copy_unmap(
        struct archi_memory *memory_dest, ///< [in] Destination memory.
        size_t offset_dest, ///< [in] Offset into destination memory in data elements.
        void *map_data_dest, ///< [in] Interface-specific data for mapping destination memory.

        struct archi_memory *memory_src, ///< [in] Source memory.
        size_t offset_src, ///< [in] Offset into source memory in data elements.
        void *map_data_src, ///< [in] Interface-specific data for mapping source memory.

        size_t num_of ///< [in] Number of data elements to copy.
);

#endif // _ARCHI_MEM_INTERFACE_FUN_H_

