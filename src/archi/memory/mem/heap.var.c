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

#include "archi/memory/mem/heap.var.h"

#include <stdlib.h> // for malloc(), aligned_alloc(), free()

ARCHI_MEMORY_ALLOC_FUNC(archi_memory_heap_alloc)
{
    (void) alloc_data;
    (void) code;

    void *allocation;

    if (alignment != 0)
        allocation = aligned_alloc(alignment, num_bytes);
    else
        allocation = malloc(num_bytes);

    return (archi_memory_alloc_info_t){.allocation = allocation};
}

ARCHI_MEMORY_FREE_FUNC(archi_memory_heap_free)
{
    free(alloc_info.allocation);
}

ARCHI_MEMORY_MAP_FUNC(archi_memory_heap_map)
{
    (void) num_bytes;
    (void) for_writing;
    (void) map_data;
    (void) code;

    return (char*)alloc_info.allocation + offset;
}

const archi_memory_interface_t archi_memory_heap_interface = {
    .alloc_fn = archi_memory_heap_alloc,
    .free_fn = archi_memory_heap_free,
    .map_fn = archi_memory_heap_map,
};

