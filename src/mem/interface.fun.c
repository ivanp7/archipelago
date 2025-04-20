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

#include "archi/mem/interface.fun.h"
#include "archi/util/size.fun.h"
#include "archi/util/size.def.h"
#include "archi/util/ref_count.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy()

struct archi_memory {
    archi_pointer_t interface; ///< Memory interface.

    archi_pointer_t allocation; ///< Current memory allocation.
    archi_pointer_t mapping;    ///< Current memory mapping.
};

archi_pointer_t
archi_memory_interface(
        struct archi_memory *memory)
{
    if (memory == NULL)
        return (archi_pointer_t){0};

    return memory->interface;
}

archi_array_layout_t
archi_memory_layout(
        struct archi_memory *memory)
{
    if (memory == NULL)
        return (archi_array_layout_t){0};

    return memory->allocation.element;
}

archi_pointer_t
archi_memory_data(
        struct archi_memory *memory)
{
    if (memory == NULL)
        return (archi_pointer_t){0};

    return memory->mapping;
}

/*****************************************************************************/

struct archi_memory*
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
    struct archi_memory *memory = malloc(sizeof(*memory));
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

    void *allocation = interface_ptr->alloc_fn(
            num_bytes, layout.alignment, alloc_data, &code_alloc);

    if (code_alloc < 0)
    {
        if (code != NULL)
            *code = code_alloc;

        free(memory);
        return NULL;
    }
    else if (allocation == NULL)
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
        .ptr = allocation,
        .element = layout,
    };

    if (code != NULL)
        *code = code_alloc;

    return memory;
}

void
archi_memory_free(
        struct archi_memory *memory)
{
    if (memory == NULL)
        return;

    if (memory->mapping.ptr != NULL)
        archi_memory_unmap(memory);

    const archi_memory_interface_t *interface_ptr = memory->interface.ptr;

    if (interface_ptr->free_fn != NULL)
        interface_ptr->free_fn(memory->allocation.ptr);

    archi_reference_count_decrement(memory->interface.ref_count);

    free(memory);
}

static
ARCHI_DESTRUCTOR_FUNC(archi_memory_auto_unmap)
{
    struct archi_memory *memory = data;

    memory->mapping.ref_count = NULL; // prevent double deallocation
    archi_memory_unmap(memory);
}

archi_pointer_t
archi_memory_map(
        struct archi_memory *memory,
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
    struct archi_reference_count *ref_count =
        archi_reference_count_alloc(archi_memory_auto_unmap, memory);
    if (ref_count == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return (archi_pointer_t){0};
    }

    // Map the memory area
    archi_status_t code_map = 0;

    const archi_memory_interface_t *interface_ptr = memory->interface.ptr;

    void *mapping = interface_ptr->map_fn(memory->allocation.ptr,
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
        struct archi_memory *memory)
{
    if ((memory == NULL) || (memory->mapping.ptr == NULL))
        return;

    const archi_memory_interface_t *interface_ptr = memory->interface.ptr;

    if (interface_ptr->unmap_fn != NULL)
        interface_ptr->unmap_fn(memory->allocation.ptr, memory->mapping.ptr);

    archi_reference_count_free(memory->mapping.ref_count);

    memory->mapping = (archi_pointer_t){0};
}

/*****************************************************************************/

archi_status_t
archi_memory_map_copy_unmap(
        struct archi_memory *memory_dest,
        size_t offset_dest,
        void *map_data_dest,

        struct archi_memory *memory_src,
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

