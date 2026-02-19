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

#include "archi/memory/api/interface.fun.h"
#include "archi/memory/api/tag.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/ref_count.fun.h"
#include "archi_base/util/size.def.h"

#include <stdlib.h> // for malloc(), free()


struct archi_memory {
    archi_rcpointer_t interface; ///< Memory interface.

    archi_rcpointer_t allocation; ///< Pointer to memory allocation.
    void *metadata; ///< Metadata for the memory allocation.

    size_t length; ///< Number of data elements.
    size_t stride; ///< Size of a data element.
    size_t alignment; ///< Data element alignment requirement.
    size_t ext_alignment; ///< Extended alignment requirement of the memory.
};

archi_rcpointer_t
archi_memory_interface(
        archi_memory_t memory)
{
    if (memory == NULL)
        return (archi_rcpointer_t){0};

    return memory->interface;
}

archi_rcpointer_t
archi_memory_allocation(
        archi_memory_t memory)
{
    if (memory == NULL)
        return (archi_rcpointer_t){0};

    return memory->allocation;
}

size_t
archi_memory_length(
        archi_memory_t memory)
{
    if (memory == NULL)
        return 0;

    return memory->length;
}

size_t
archi_memory_stride(
        archi_memory_t memory)
{
    if (memory == NULL)
        return 0;

    return memory->stride;
}

size_t
archi_memory_size(
        archi_memory_t memory)
{
    if (memory == NULL)
        return 0;

    return memory->length * memory->stride;
}

size_t
archi_memory_alignment(
        archi_memory_t memory)
{
    if (memory == NULL)
        return 0;

    return memory->alignment;
}

size_t
archi_memory_ext_alignment(
        archi_memory_t memory)
{
    if (memory == NULL)
        return 0;

    return memory->ext_alignment;
}

/*****************************************************************************/

static
ARCHI_DESTRUCTOR_FUNC(archi_memory_deallocator)
{
    archi_memory_t memory = data;

    const archi_memory_interface_t *interface_ptr = memory->interface.cptr;

    // Free the memory
    if (interface_ptr->free_fn != NULL)
    {
        archi_memory_alloc_info_t alloc_info = {
            .allocation = {
                .ptr = memory->allocation.ptr,
                .writable = ARCHI_POINTER_TO_WRITABLE_DATA(memory->allocation.attr),
            },
            .metadata = memory->metadata,
        };
        archi_pointer_attr_unpk__cdata(memory->allocation.attr, &alloc_info.allocation.tag, NULL);

        /*********************************/
        interface_ptr->free_fn(alloc_info);
        /*********************************/
    }

    // Decrement the reference count of the interface
    archi_reference_count_decrement(memory->interface.ref_count);

    // Destroy the memory object
    free(memory);
}

archi_memory_t
archi_memory_allocate(
        archi_rcpointer_t interface,
        void *alloc_data,

        size_t length,
        size_t stride,
        size_t alignment,
        size_t ext_alignment,

        ARCHI_ERROR_PARAM_DECL)
{
    // Perform necessary checks
    ARCHI_ERROR_VAR(error);

    if (!archi_pointer_valid(interface.p, &error))
    {
        ARCHI_ERROR_SET(error.code, "memory interface pointer is invalid: %s", error.message);
        return NULL;
    }

    if (!archi_pointer_attr_compatible(interface.attr,
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory interface pointer attributes are incorrect");
        return NULL;
    }
    else if (interface.cptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory interface pointer is NULL");
        return NULL;
    }
    else if (ARCHI_POINTER_TO_STACK(interface.attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory interface is on stack");
        return NULL;
    }

    if (ext_alignment == 0)
        ext_alignment = alignment;

    archi_pointer_attr_t attr = archi_pointer_attr__pdata(length, stride, alignment, &error);

    if (length == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory parameter is invalid: length is zero");
        return NULL;
    }
    else if (attr == (archi_pointer_attr_t)-1)
    {
        ARCHI_ERROR_SET(error.code, "memory parameter is invalid: %s", error.message);
        return NULL;
    }
    else if (!ARCHI_ALIGNMENT_VALID(ext_alignment))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory parameter is invalid: extended alignment (%#zx) is not a power of two",
                ext_alignment);
        return NULL;
    }
    else if (ext_alignment < alignment)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory parameter is invalid: extended alignment (%#zx) is less than alignment (%#zx)",
                ext_alignment, alignment);
        return NULL;
    }

    // Check pointer to interface
    const archi_memory_interface_t *interface_ptr = interface.cptr;
    if (interface_ptr->alloc_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory interface doesn't have alloc_fn()");
        return NULL;
    }

    // Allocate the memory object
    archi_memory_t memory = malloc(sizeof(*memory));
    if (memory == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate memory object");
        return NULL;
    }

    *memory = (struct archi_memory){
        .interface = interface,

        .length = length,
        .stride = stride,
        .alignment = alignment,
        .ext_alignment = ext_alignment,
    };

    // Allocate the reference counter
    archi_reference_count_t ref_count =
        archi_reference_count_alloc(archi_memory_deallocator, memory);
    if (ref_count == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate memory reference counter");
        goto failure;
    }

    // Allocate the memory itself
    ARCHI_ERROR_VAR_UNSET(&error);
    /*************************************************************/
    archi_memory_alloc_info_t alloc_info = interface_ptr->alloc_fn(
            length * stride, ext_alignment, alloc_data, &error);
    /*************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (alloc_info.allocation.ptr == NULL)
    {
        if (error.code == 0)
            ARCHI_ERROR_SET(ARCHI__ECONTRACT, "alloc_fn() returned zero status code on failure");
        goto failure;
    }

    // Store the allocation into the object
    memory->allocation = (archi_rcpointer_t){
        .ptr = alloc_info.allocation.ptr,
        .attr = (alloc_info.allocation.writable ?
                ARCHI_POINTER_TYPE__DATA_WRITABLE : ARCHI_POINTER_TYPE__DATA_READONLY) |
            (alloc_info.allocation.tag ?
             archi_pointer_attr__cdata(alloc_info.allocation.tag) : attr),
        .ref_count = ref_count,
    };
    memory->metadata = alloc_info.metadata;

    // Increase the reference count of the interface
    archi_reference_count_increment(interface.ref_count);

    return memory;

failure:
    archi_reference_count_free(ref_count);
    free(memory);

    return NULL;
}

void
archi_memory_free(
        archi_memory_t memory)
{
    if (memory == NULL)
        return;

    // Decrement the reference count of the memory allocation
    archi_reference_count_decrement(memory->allocation.ref_count);
}

/*****************************************************************************/

struct archi_memory_mapping {
    archi_memory_t memory; ///< Backing memory object.

    archi_rcpointer_t pointer; ///< Pointer to the mapped memory region.
    void *metadata; ///< Metadata for the memory mapping.

    size_t offset; ///< Offset of the mapped region.
};

archi_memory_t
archi_memory_mapping_memory(
        archi_memory_mapping_t mapping)
{
    if (mapping == NULL)
        return NULL;

    return mapping->memory;
}

archi_rcpointer_t
archi_memory_mapping_pointer(
        archi_memory_mapping_t mapping)
{
    if (mapping == NULL)
        return (archi_rcpointer_t){0};

    return mapping->pointer;
}

size_t
archi_memory_mapping_offset(
        archi_memory_mapping_t mapping)
{
    if (mapping == NULL)
        return 0;

    return mapping->offset;
}

/*****************************************************************************/

static
ARCHI_DESTRUCTOR_FUNC(archi_memory_mapping_destructor)
{
    archi_memory_mapping_t mapping = data;
    archi_memory_t memory = mapping->memory;

    const archi_memory_interface_t *interface_ptr = memory->interface.cptr;

    // Unmap the memory
    if (interface_ptr->unmap_fn != NULL)
    {
        archi_memory_alloc_info_t alloc_info = {
            .allocation = {
                .ptr = memory->allocation.ptr,
                .writable = ARCHI_POINTER_TO_WRITABLE_DATA(memory->allocation.attr),
            },
            .metadata = memory->metadata,
        };
        archi_pointer_attr_unpk__cdata(memory->allocation.attr, &alloc_info.allocation.tag, NULL);

        archi_memory_map_info_t map_info = {
            .mapping = {
                .ptr = mapping->pointer.ptr,
                .writable = ARCHI_POINTER_TO_WRITABLE_DATA(mapping->pointer.attr),
            },
            .metadata = mapping->metadata,
        };

        /********************************************/
        interface_ptr->unmap_fn(alloc_info, map_info);
        /********************************************/
    }

    // Decrement the reference count of the memory object
    archi_reference_count_decrement(memory->allocation.ref_count);

    // Destroy the memory mapping object
    free(mapping);
}

archi_memory_mapping_t
archi_memory_map(
        archi_memory_t memory,
        void *map_data,

        size_t offset,
        size_t length,

        ARCHI_ERROR_PARAM_DECL)
{
    // Perform necessary checks
    if (memory == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory object is NULL");
        return NULL;
    }
    else if (offset >= memory->length)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory mapping offset (%zu) out of range (memory length = %zu)",
                offset, memory->length);
        return NULL;
    }

    if (length == 0)
        length = memory->length - offset;
    else if (offset + length > memory->length)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory mapping offset+length (%zu + %zu) out of range (memory length = %zu)",
                offset, length, memory->length);
        return NULL;
    }

    const archi_memory_interface_t *interface_ptr = memory->interface.cptr;
    if (interface_ptr->map_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory interface doesn't have map_fn()");
        return NULL;
    }

    // Allocate the memory mapping object
    archi_memory_mapping_t mapping = malloc(sizeof(*mapping));
    if (mapping == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate memory mapping object");
        return NULL;
    }

    *mapping = (struct archi_memory_mapping){
        .memory = memory,
        .offset = offset,
    };

    // Allocate the reference counter
    archi_reference_count_t ref_count =
        archi_reference_count_alloc(archi_memory_mapping_destructor, mapping);
    if (ref_count == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate memory mapping refcounter");
        goto failure;
    }

    // Map the memory region
    archi_memory_alloc_info_t alloc_info = {
        .allocation = {
            .ptr = memory->allocation.ptr,
            .writable = ARCHI_POINTER_TO_WRITABLE_DATA(memory->allocation.attr),
        },
        .metadata = memory->metadata,
    };
    archi_pointer_attr_unpk__cdata(memory->allocation.attr, &alloc_info.allocation.tag, NULL);

    ARCHI_ERROR_VAR(error);

    /**************************************************************************/
    archi_memory_map_info_t map_info = interface_ptr->map_fn(alloc_info,
            offset * memory->stride, length * memory->stride, map_data, &error);
    /**************************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (map_info.mapping.ptr == NULL)
    {
        if (error.code == 0)
            ARCHI_ERROR_SET(ARCHI__ECONTRACT, "map_fn() returned zero status code on failure");
        goto failure;
    }

    // Store the mapping into the object
    mapping->pointer = (archi_rcpointer_t){
        .ptr = map_info.mapping.ptr,
        .attr = (map_info.mapping.writable ?
                ARCHI_POINTER_TYPE__DATA_WRITABLE : ARCHI_POINTER_TYPE__DATA_READONLY) |
            archi_pointer_attr__pdata(length, memory->stride, memory->alignment, NULL),
        .ref_count = ref_count,
    };
    mapping->metadata = map_info.metadata;

    // Increase the reference count of the memory object
    archi_reference_count_increment(memory->allocation.ref_count);

    return mapping;

failure:
    archi_reference_count_free(ref_count);
    free(mapping);

    return NULL;
}

void
archi_memory_unmap(
        archi_memory_mapping_t mapping)
{
    if (mapping == NULL)
        return;

    archi_reference_count_decrement(mapping->pointer.ref_count);
}

