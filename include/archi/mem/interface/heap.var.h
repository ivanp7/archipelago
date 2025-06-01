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
 * @brief Memory interface for heap memory.
 */

#pragma once
#ifndef _ARCHI_MEM_INTERFACE_HEAP_VAR_H_
#define _ARCHI_MEM_INTERFACE_HEAP_VAR_H_

#include "archi/mem/interface.typ.h"

/**
 * @brief Heap memory allocation function.
 *
 * Allocates a block of memory from the heap.
 * If layout.alignment == 0, malloc() is used; otherwise,
 * aligned_alloc() is used with the specified alignment.
 * @p alloc_data is ignored. @p code is never set.
 */
ARCHI_MEMORY_ALLOC_FUNC(archi_memory_heap_alloc);

/**
 * @brief Heap memory deallocation function.
 */
ARCHI_MEMORY_FREE_FUNC(archi_memory_heap_free);

/**
 * @brief Heap memory mapping function.
 *
 * This function does nothing but calculating the pointer targeting the specified area.
 * @p code is never set.
 */
ARCHI_MEMORY_MAP_FUNC(archi_memory_heap_map);

/**
 * @brief Heap memory interface.
 */
extern
const archi_memory_interface_t archi_memory_heap_interface;

#endif // _ARCHI_MEM_INTERFACE_HEAP_VAR_H_

