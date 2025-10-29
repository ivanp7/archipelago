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
 * @brief Context interface for lock-free queues.
 */

#include "archi/thread/ctx/lfqueue.var.h"
#include "archi/thread/api/lfqueue.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <stdalign.h>

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__thread_lfqueue)
{
    // Parse parameters
    archi_thread_lfqueue_alloc_params_t lfqueue_alloc_params = {0};
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "params", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_thread_lfqueue_alloc_params_t)},
            {.name = "capacity", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "element_size", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "element_alignment", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            lfqueue_alloc_params = *(archi_thread_lfqueue_alloc_params_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            lfqueue_alloc_params.capacity = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            lfqueue_alloc_params.element.size = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            lfqueue_alloc_params.element.alignment = *(size_t*)parsed[index].value.ptr;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_thread_lfqueue_t lfqueue = archi_thread_lfqueue_alloc(lfqueue_alloc_params, ARCHI_ERROR_PARAMETER);
    if (lfqueue == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = lfqueue,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__opaque_data(ARCHI_POINTER_DATA_TAG__THREAD_LFQUEUE),
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__thread_lfqueue)
{
    archi_thread_lfqueue_free(context->ptr);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__thread_lfqueue)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("capacity", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t capacity = archi_thread_lfqueue_capacity(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &capacity,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("element.size", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t element_size = archi_thread_lfqueue_element_size(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &element_size,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("element.alignment", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t element_alignment = archi_thread_lfqueue_element_alignment(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &element_alignment,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__thread_lfqueue = {
    .init_fn = archi_context_init__thread_lfqueue,
    .final_fn = archi_context_final__thread_lfqueue,
    .eval_fn = archi_context_eval__thread_lfqueue,
};

