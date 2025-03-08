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

#include "archi/util/memory.fun.h"

#include <stdlib.h> // for aligned_alloc(), free()
#include <stdalign.h> // for alignof()

void*
archi_memory_allocate(
        archi_memory_alloc_config_t *config,

        const archi_memory_interface_t *interface)
{
    if (config == NULL)
        return NULL;
    else if ((interface == NULL) || (interface->alloc_fn == NULL))
        return NULL;

    return interface->alloc_fn(config);
}

void
archi_memory_free(
        void *restrict memory,
        void *restrict param,

        const archi_memory_interface_t *interface)
{
    if (memory == NULL)
        return;
    else if (interface == NULL)
        return;

    if (interface->free_fn != NULL)
        interface->free_fn(memory, param);
}

bool
archi_memory_map(
        void *restrict memory,
        archi_memory_map_config_t *restrict config,

        const archi_memory_interface_t *interface)
{
    if ((memory == NULL) || (config == NULL))
        return NULL;
    else if (interface == NULL)
        return NULL;

    if (interface->map_fn != NULL)
        return interface->map_fn(memory, config);
    else
        return true;
}

bool
archi_memory_unmap(
        void *restrict memory,
        archi_memory_map_config_t *restrict config,

        const archi_memory_interface_t *interface)
{
    if ((memory == NULL) || (config == NULL))
        return NULL;
    else if (interface == NULL)
        return NULL;

    if (interface->unmap_fn != NULL)
        return interface->unmap_fn(memory, config);
    else
        return true;
}

/*****************************************************************************/

ARCHI_MEMORY_ALLOC_FUNC(archi_memory_heap_alloc_func)
{
    if (config->num_bytes == 0)
        return NULL;

    size_t alignment = config->alignment;
    if (alignment == 0)
        alignment = alignof(max_align_t);

    return aligned_alloc(alignment, config->num_bytes);
}

ARCHI_MEMORY_FREE_FUNC(archi_memory_heap_free_func)
{
    (void) param;

    free(memory);
}

ARCHI_MEMORY_MAP_FUNC(archi_memory_heap_map_func)
{
    (void) memory;
    (void) config;

    return true; // heap memory does not need to be mapped/unmapped
}

ARCHI_MEMORY_UNMAP_FUNC(archi_memory_heap_unmap_func)
{
    (void) memory;
    (void) config;

    return true; // heap memory does not need to be mapped/unmapped
}

const archi_memory_interface_t archi_memory_heap_interface = {
    .alloc_fn = archi_memory_heap_alloc_func,
    .free_fn = archi_memory_heap_free_func,
    .map_fn = archi_memory_heap_map_func,
    .unmap_fn = archi_memory_heap_unmap_func,
};

