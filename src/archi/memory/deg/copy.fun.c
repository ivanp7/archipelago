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
 * @brief DEG node function for copying data between memory objects.
 */

#include "archi/memory/deg/copy.fun.h"
#include "archi/memory/deg/copy.typ.h"
#include "archi/memory/api/interface.fun.h"
#include "archipelago/base/pointer.fun.h"

#include <string.h> // for memmove()

ARCHI_DEG_NODE_FUNCTION(archi_deg_node__memory_map_copy_unmap)
{
    archi_deg_node_data__memory_map_copy_unmap_t *copy = data;
    if (copy == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory copy operation data is NULL");
        return;
    }

    // Perform necessary checks
    if (copy->dest.memory == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination memory is NULL");
        return;
    }
    else if (copy->src.memory == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source memory is NULL");
        return;
    }

    size_t copy_stride = archi_memory_stride(copy->src.memory);
    {
        if (archi_memory_stride(copy->dest.memory) != copy_stride)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination (%zu) and source (%zu) memory strides are not equal",
                    archi_memory_stride(copy->dest.memory), copy_stride);
            return;
        }
        else if (archi_memory_alignment(copy->dest.memory) != archi_memory_alignment(copy->src.memory))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination (%zu) and source (%zu) memory alignments are not equal",
                    archi_memory_alignment(copy->dest.memory), archi_memory_alignment(copy->src.memory));
            return;
        }
    }

    size_t copy_length = copy->length;
    {
        size_t dest_length = archi_memory_length(copy->dest.memory);
        size_t src_length = archi_memory_length(copy->src.memory);

        if (copy_length != 0)
        {
            if (copy->src.offset + copy_length > src_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source memory region (offset = %zu, length = %zu) is out of bounds (length = %zu)",
                        copy->src.offset, copy_length, src_length);
                return;
            }
        }
        else // copy everything to the end of the source memory
        {
            if (copy->src.offset >= src_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source memory region (offset = %zu) is out of bounds (length = %zu)",
                        copy->src.offset, src_length);
                return;
            }

            copy_length = src_length - copy->src.offset;
        }

        if (copy->dest.offset + copy_length > dest_length)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination memory region (offset = %zu, length = %zu) is out of bounds (length = %zu)",
                    copy->dest.offset, copy_length, dest_length);
            return;
        }
    }

    archi_error_t error;

    // Map source memory
    ARCHI_ERROR_RESET_VAR(&error);
    archi_memory_mapping_t src_mapping = archi_memory_map(copy->src.memory,
            copy->src.map_data, copy->src.offset, copy_length, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (src_mapping == NULL)
    {
        ARCHI_ERROR_SET(error.code, "couldn't map source memory: %s", error.message);
        return;
    }

    // Map destination memory
    ARCHI_ERROR_RESET_VAR(&error);
    archi_memory_mapping_t dest_mapping = archi_memory_map(copy->dest.memory,
            copy->dest.map_data, copy->dest.offset, copy_length, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (dest_mapping == NULL)
    {
        archi_memory_unmap(src_mapping);

        ARCHI_ERROR_SET(error.code, "couldn't map destination memory: %s", error.message);
        return;
    }

    // Copy data
    memmove(archi_memory_mapping_pointer(dest_mapping).ptr,
            archi_memory_mapping_pointer(src_mapping).ptr,
            copy_length * copy_stride);

    // Unmap memory
    archi_memory_unmap(dest_mapping);
    archi_memory_unmap(src_mapping);

    ARCHI_ERROR_RESET();
}

