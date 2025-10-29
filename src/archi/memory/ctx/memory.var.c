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
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__memory)
{
    // Parse parameters
    archi_rcpointer_t interface = {0};
    void *alloc_data = NULL;
    size_t length = 0;
    size_t stride = 0;
    size_t alignment = 0;
    size_t overalignment = 0;
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "interface", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_memory_interface_t)},
            {.name = "alloc_data", .value.attr = archi_pointer_attr__opaque_data(0)},
            {.name = "length", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "stride", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "alignment", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "overalignment", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            interface = parsed[index].value;
        index++;
        if (parsed[index].value_set)
            alloc_data = parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            length = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            stride = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            alignment = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            overalignment = *(size_t*)parsed[index].value.ptr;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_memory_t memory = archi_memory_allocate(interface, alloc_data,
            length, stride, alignment, overalignment, ARCHI_ERROR_PARAMETER);
    if (memory == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = memory,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__opaque_data(ARCHI_POINTER_DATA_TAG__MEMORY),
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
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
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
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
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
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
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
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("overalignment", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t overalignment = archi_memory_overalignment(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &overalignment,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
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

