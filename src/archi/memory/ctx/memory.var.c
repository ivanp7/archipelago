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
 * @brief Context interface for memory objects.
 */

#include "archi/memory/ctx/memory.var.h"
#include "archi/memory/api/interface.fun.h"
#include "archi/memory/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__memory)
{
    // Parse parameters
    archi_rcpointer_t interface = {0};
    archi_pointer_t alloc_data = {0};
    size_t length = 0, stride = 0, alignment = 0, ext_alignment = 0;
    {
        archi_plist_param_t parsed[] = {
            {.name = "interface",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__MEMORY_INTERFACE)}},
                .assign = {archi_plist_assign__rcpointer, &interface, sizeof(interface)}},
            {.name = "alloc_data",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                .assign = {archi_plist_assign__pointer, &alloc_data, sizeof(alloc_data)}},
            {.name = "length",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &length, sizeof(length)}},
            {.name = "stride",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &stride, sizeof(stride)}},
            {.name = "alignment",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &alignment, sizeof(alignment)}},
            {.name = "ext_alignment",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &ext_alignment, sizeof(ext_alignment)}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_memory_t memory = archi_memory_allocate(interface, alloc_data,
            length, stride, alignment, ext_alignment, ARCHI_ERROR_PARAM);
    if (memory == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = memory,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__MEMORY),
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__memory)
{
    archi_memory_free(context->ptr);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__memory)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("interface", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(archi_memory_interface(context->ptr));
    }
    else if (ARCHI_STRING_COMPARE("allocation", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(archi_memory_allocation(context->ptr));
    }
    else if (ARCHI_STRING_COMPARE("length", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t length = archi_memory_length(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &length,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("stride", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t stride = archi_memory_stride(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &stride,
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

        size_t size = archi_memory_size(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &size,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("alignment", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t alignment = archi_memory_alignment(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &alignment,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("ext_alignment", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t ext_alignment = archi_memory_ext_alignment(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &ext_alignment,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__memory = {
    .init_fn = archi_context_init__memory,
    .final_fn = archi_context_final__memory,
    .eval_fn = archi_context_eval__memory,
};

