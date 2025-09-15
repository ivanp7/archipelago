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

#include "archi/mem/api/interface.fun.h"
#include "archipelago/util/size.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy()

struct archi_memory {
    archi_pointer_t interface; ///< Memory interface.

    archi_pointer_t allocation; ///< Current memory allocation.
    void *metadata; ///< Metadata for the current memory allocation.

    archi_pointer_t mapping; ///< Current memory mapping.
};

archi_pointer_t
archi_memory_interface(
        archi_memory_t memory)
{
    if (memory == NULL)
        return (archi_pointer_t){0};

    return memory->interface;
}

archi_pointer_t
archi_memory_allocation(
        archi_memory_t memory)
{
    if (memory == NULL)
        return (archi_pointer_t){0};

    return memory->allocation;
}

archi_pointer_t
archi_memory_mapping(
        archi_memory_t memory)
{
    if (memory == NULL)
        return (archi_pointer_t){0};

    return memory->mapping;
}

/*****************************************************************************/

archi_memory_t
archi_memory_allocate(
        archi_pointer_t interface,
        void *alloc_data,

        archi_array_layout_t layout,

        archi_status_t *code)
{
    if ((interface.flags & ARCHI_POINTER_FLAG_FUNCTION) || (interface.ptr == NULL))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    const archi_memory_interface_t *interface_ptr = interface.ptr;
    if ((interface_ptr->alloc_fn == NULL) || (interface_ptr->map_fn == NULL))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EINTERFACE;

        return NULL;
    }

    // Calculate the total number of bytes requested
    size_t num_bytes = archi_size_array(layout);
    if (num_bytes == 0) // invalid layout
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    // Allocate the memory object
    archi_memory_t memory = malloc(sizeof(*memory));
    if (memory == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return NULL;
    }

    *memory = (struct archi_memory){
        .interface = interface,
    };

    // Allocate the memory itself
    archi_status_t code_alloc = 0;

    archi_memory_alloc_info_t alloc_info = interface_ptr->alloc_fn(
            num_bytes, layout.alignment, alloc_data, &code_alloc);

    if (code_alloc < 0)
    {
        if (code != NULL)
            *code = code_alloc;

        free(memory);
        return NULL;
    }
    else if (alloc_info.allocation == NULL)
    {
        if (code != NULL)
            *code = (code_alloc > 0) ? code_alloc : ARCHI_STATUS_ENOMEMORY;

        free(memory);
        return NULL;
    }

    // Increase the reference count of the interface
    archi_reference_count_increment(interface.ref_count);

    // Store the allocation into the object
    memory->allocation = (archi_pointer_t){
        .ptr = alloc_info.allocation,
        .element = layout,
    };

    memory->metadata = alloc_info.metadata;

    if (code != NULL)
        *code = code_alloc;

    return memory;
}

void
archi_memory_free(
        archi_memory_t memory)
{
    if (memory == NULL)
        return;

    if (memory->mapping.ptr != NULL)
        archi_memory_unmap(memory);

    const archi_memory_interface_t *interface_ptr = memory->interface.ptr;

    if (interface_ptr->free_fn != NULL)
    {
        archi_memory_alloc_info_t alloc_info = {
            .allocation = memory->allocation.ptr,
            .metadata = memory->metadata,
        };

        interface_ptr->free_fn(alloc_info);
    }

    archi_reference_count_decrement(memory->interface.ref_count);

    free(memory);
}

static
ARCHI_DESTRUCTOR_FUNC(archi_memory_mapping_destructor)
{
    archi_memory_t memory = data;

    const archi_memory_interface_t *interface_ptr = memory->interface.ptr;

    if (interface_ptr->unmap_fn != NULL)
    {
        archi_memory_alloc_info_t alloc_info = {
            .allocation = memory->allocation.ptr,
            .metadata = memory->metadata,
        };

        interface_ptr->unmap_fn(alloc_info, memory->mapping.ptr);
    }

    memory->mapping = (archi_pointer_t){0};
}

archi_pointer_t
archi_memory_map(
        archi_memory_t memory,
        void *map_data,

        size_t offset,
        size_t num_of,
        bool writeable,

        archi_status_t *code)
{
    if ((memory == NULL) || (memory->mapping.ptr != NULL) ||
            (offset >= memory->allocation.element.num_of))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return (archi_pointer_t){0};
    }

    // Prepare layout and calculate sizes
    if (num_of == 0)
        num_of = memory->allocation.element.num_of - offset;
    else if (offset + num_of > memory->allocation.element.num_of)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return (archi_pointer_t){0};
    }

    archi_array_layout_t layout = {
        .num_of = num_of,
        .size = memory->allocation.element.size,
        .alignment = memory->allocation.element.alignment,
    };

    size_t num_bytes = archi_size_array(layout);
    size_t element_size = ARCHI_SIZE_PADDED(layout.size, layout.alignment);

    // Allocate the reference counter
    archi_reference_count_t ref_count =
        archi_reference_count_alloc(archi_memory_mapping_destructor, memory);
    if (ref_count == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return (archi_pointer_t){0};
    }

    // Map the memory area
    archi_status_t code_map = 0;

    const archi_memory_interface_t *interface_ptr = memory->interface.ptr;

    archi_memory_alloc_info_t alloc_info = {
        .allocation = memory->allocation.ptr,
        .metadata = memory->metadata,
    };

    void *mapping = interface_ptr->map_fn(alloc_info,
            offset * element_size, num_bytes, writeable, map_data, &code_map);

    if (code_map < 0)
    {
        if (code != NULL)
            *code = code_map;

        free(ref_count);
        return (archi_pointer_t){0};
    }
    else if (mapping == NULL)
    {
        if (code != NULL)
            *code = (code_map > 0) ? code_map : ARCHI_STATUS_ERESOURCE;

        free(ref_count);
        return (archi_pointer_t){0};
    }

    // Store the allocation into the object
    memory->mapping = (archi_pointer_t){
        .ptr = mapping,
        .ref_count = ref_count,
        .flags = writeable ? ARCHI_POINTER_FLAG_WRITABLE : 0,
        .element = layout,
    };

    if (code != NULL)
        *code = code_map;

    return memory->mapping;
}

void
archi_memory_unmap(
        archi_memory_t memory)
{
    if (memory == NULL)
        return;

    archi_reference_count_decrement(memory->mapping.ref_count);
}

/*****************************************************************************/

archi_status_t
archi_memory_map_copy_unmap(
        archi_memory_t memory_dest,
        size_t offset_dest,
        void *map_data_dest,

        archi_memory_t memory_src,
        size_t offset_src,
        void *map_data_src,

        size_t num_of)
{
    if ((memory_dest == NULL) || (memory_src == NULL) || (memory_dest == memory_src))
        return ARCHI_STATUS_EMISUSE;
    else if ((memory_dest->mapping.ptr != NULL) || (memory_src->mapping.ptr != NULL))
        return ARCHI_STATUS_EMISUSE;
    else if (memory_dest->allocation.element.size != memory_src->allocation.element.size)
        return ARCHI_STATUS_EMISUSE;
    else if (ARCHI_SIZE_PADDED(memory_dest->allocation.element.size,
                memory_dest->allocation.element.alignment) !=
            ARCHI_SIZE_PADDED(memory_src->allocation.element.size,
                memory_src->allocation.element.alignment))
        return ARCHI_STATUS_EMISUSE;

    if (num_of == 0)
        return 0;

    if ((offset_dest >= memory_dest->allocation.element.num_of) ||
            (offset_src >= memory_src->allocation.element.num_of))
        return ARCHI_STATUS_EMISUSE;
    else if ((offset_dest + num_of > memory_dest->allocation.element.num_of) ||
            (offset_src + num_of > memory_src->allocation.element.num_of))
        return ARCHI_STATUS_EMISUSE;

    size_t num_bytes = archi_size_array((archi_array_layout_t){ // number of bytes to copy
            .num_of = num_of,
            .size = memory_dest->allocation.element.size,
            .alignment = memory_dest->allocation.element.alignment,
        });

    archi_status_t code = 0;

    // Map source memory
    archi_pointer_t src = archi_memory_map(memory_src, map_data_src,
            offset_src, num_of, false, &code);
    if (src.ptr == NULL)
        return code;

    // Map destination memory
    archi_pointer_t dest = archi_memory_map(memory_dest, map_data_dest,
            offset_dest, num_of, true, &code);
    if (dest.ptr == NULL)
    {
        archi_memory_unmap(memory_src);
        return code;
    }

    // Copy data
    memcpy(dest.ptr, src.ptr, num_bytes);

    // Unmap memory
    archi_memory_unmap(memory_dest);
    archi_memory_unmap(memory_src);

    return 0;
}

