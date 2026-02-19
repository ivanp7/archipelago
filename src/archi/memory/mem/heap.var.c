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
 * @brief Memory interface for heap memory.
 */

#include "archi/memory/mem/heap.var.h"

#include <stdlib.h> // for aligned_alloc(), free()


static
ARCHI_MEMORY_ALLOC_FUNC(archi_memory_alloc__heap)
{
    (void) alloc_data;

    void *allocation = aligned_alloc(alignment, num_bytes);
    if (allocation == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate memory on heap (%zu bytes, alignment = %#zx)",
                num_bytes, alignment);
        return (archi_memory_alloc_info_t){0};
    }

    ARCHI_ERROR_RESET();
    return (archi_memory_alloc_info_t){.allocation = {.ptr = allocation, .writable = true}};
}

static
ARCHI_MEMORY_FREE_FUNC(archi_memory_free__heap)
{
    free(alloc_info.allocation.ptr);
}

static
ARCHI_MEMORY_MAP_FUNC(archi_memory_map__heap)
{
    (void) num_bytes;
    (void) map_data;

    ARCHI_ERROR_RESET();
    return (archi_memory_map_info_t){.mapping = {
        .ptr = (char*)alloc_info.allocation.ptr + offset,
        .writable = true,
    }};
}

const archi_memory_interface_t
archi_memory_interface__heap = {
    .alloc_fn = archi_memory_alloc__heap,
    .free_fn = archi_memory_free__heap,
    .map_fn = archi_memory_map__heap,
};

