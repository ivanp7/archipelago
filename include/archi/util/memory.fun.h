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
#ifndef _ARCHI_UTIL_MEMORY_FUN_H_
#define _ARCHI_UTIL_MEMORY_FUN_H_

#include "archi/util/memory.typ.h"

/**
 * @brief Allocate memory.
 *
 * @return Newly allocated memory, or NULL in case of failure.
 */
void*
archi_memory_allocate(
        archi_memory_alloc_config_t *config, ///< [in] Memory allocation parameters.

        const archi_memory_interface_t *interface ///< [in] Memory interface.
);

/**
 * @brief Deallocate memory.
 */
bool
archi_memory_free(
        void *restrict memory, ///< [in] Memory to deallocate.
        archi_memory_alloc_config_t *restrict config, ///< [in] Memory allocation parameters.

        const archi_memory_interface_t *interface ///< [in] Memory interface.
);

/**
 * @brief Map memory region for reading/writing.
 *
 * @return True if succeed, otherwise false.
 */
bool
archi_memory_map(
        void *restrict memory, ///< [in] Memory to map.
        archi_memory_map_config_t *restrict config, ///< [in] Memory mapping parameters.

        const archi_memory_interface_t *interface ///< [in] Memory interface.
);

/**
 * @brief Unmap memory region.
 *
 * @return True if succeed, otherwise false.
 */
bool
archi_memory_unmap(
        void *restrict memory, ///< [in] Memory to unmap.
        archi_memory_map_config_t *restrict config, ///< [in] Memory mapping parameters.

        const archi_memory_interface_t *interface ///< [in] Memory interface.
);

/*****************************************************************************/

ARCHI_MEMORY_ALLOC_FUNC(archi_memory_heap_alloc_func); ///< Heap memory allocation function.
ARCHI_MEMORY_FREE_FUNC(archi_memory_heap_free_func);   ///< Heap memory deallocation function.
ARCHI_MEMORY_MAP_FUNC(archi_memory_heap_map_func);     ///< Heap memory mapping function.
ARCHI_MEMORY_UNMAP_FUNC(archi_memory_heap_unmap_func); ///< Heap memory unmapping function.

extern
const archi_memory_interface_t archi_memory_heap_interface; ///< Heap memory interface.

#endif // _ARCHI_UTIL_MEMORY_FUN_H_

