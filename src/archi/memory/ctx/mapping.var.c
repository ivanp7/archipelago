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
 * @brief Context interface for memory mapping objects.
 */

#include "archi/memory/ctx/mapping.var.h"
#include "archi/memory/api/interface.fun.h"
#include "archi/memory/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


struct archi_context_data__memory_mapping {
    archi_rcpointer_t mapping;

    size_t offset; ///< Offset of mapped region.
    size_t length; ///< Length of mapped region.
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__memory_mapping)
{
    // Parse parameters
    archi_rcpointer_t memory = {0};
    archi_pointer_t map_data = {0};
    size_t offset = 0, length = 0;
    {
        archi_plist_param_t parsed[] = {
            {.name = "memory",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__MEMORY)}},
                .assign = {archi_plist_assign__rcpointer, &memory, sizeof(memory)}},
            {.name = "map_data",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                .assign = {archi_plist_assign__pointer, &map_data, sizeof(map_data)}},
            {.name = "offset",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &offset, sizeof(offset)}},
            {.name = "length",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &length, sizeof(length)}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Construct the context
    struct archi_context_data__memory_mapping *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_memory_mapping_t mapping = archi_memory_map(memory.ptr,
            map_data, offset, length, ARCHI_ERROR_PARAM);
    if (mapping == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (struct archi_context_data__memory_mapping){
        .mapping = {
            .ptr = mapping,
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__MEMORY_MAPPING),
        },
        .offset = offset,
        .length = length,
    };

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__memory_mapping)
{
    struct archi_context_data__memory_mapping *context_data =
        (struct archi_context_data__memory_mapping*)context;

    archi_memory_unmap(context_data->mapping.ptr);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__memory_mapping)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__memory_mapping *context_data =
        (struct archi_context_data__memory_mapping*)context;

    if (ARCHI_STRING_COMPARE("memory", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_memory_t memory = archi_memory_mapping_memory(context_data->mapping.ptr);

        archi_rcpointer_t value = {
            .ptr = memory,
            .ref_count = archi_memory_allocation(memory).ref_count,
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__MEMORY),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("ptr", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(archi_memory_mapping_pointer(context_data->mapping.ptr));
    }
    else if (ARCHI_STRING_COMPARE("offset", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t offset = context_data->offset;

        archi_rcpointer_t value = {
            .ptr = &offset,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("length", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t length = context_data->length;

        archi_rcpointer_t value = {
            .ptr = &length,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("size", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_memory_t memory = archi_memory_mapping_memory(context_data->mapping.ptr);
        size_t size = context_data->length * archi_memory_stride(memory);

        archi_rcpointer_t value = {
            .ptr = &size,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__memory_mapping = {
    .init_fn = archi_context_init__memory_mapping,
    .final_fn = archi_context_final__memory_mapping,
    .eval_fn = archi_context_eval__memory_mapping,
};

