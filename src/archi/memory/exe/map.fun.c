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
 * @brief DEG operation function for mapping/unmapping memory.
 */

#include "archi/memory/exe/map.fun.h"
#include "archi/memory/exe/map.typ.h"
#include "archi/memory/api/interface.fun.h"

#include <string.h> // for memcpy(), memmove()


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__memory_map_unmap_wrapper)
{
    const archi_dexgraph_op_data__memory_map_unmap_wrapper_t *wrapper = data;

    if (wrapper == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "memory map/unmap wrapper data is NULL");
        return;
    }
    else if (wrapper->memory == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "mapped memory object is NULL");
        return;
    }
    else if (wrapper->map_length == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "mapped area length is 0");
        return;
    }
    else if (wrapper->target.function == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target operation function is NULL");
        return;
    }
    else if (wrapper->ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "location to write mapped memory pointer to is NULL");
        return;
    }

    // Map the memory
    archi_memory_mapping_t mapping = archi_memory_map(wrapper->memory,
            wrapper->map_data, wrapper->map_offset, wrapper->map_length, ARCHI_ERROR_PARAM);

    if (mapping == NULL)
        return;

    // Set the mapped memory pointer
    *wrapper->ptr = archi_memory_mapping_pointer(mapping).ptr;

    // Call the target operation function
    wrapper->target.function(wrapper->target.data, ARCHI_ERROR_PARAM);

    // Unset the mapped memory pointer
    *wrapper->ptr = NULL;

    // Unmap the memory
    archi_memory_unmap(mapping);
}

