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
 * @brief Context interface for thread groups.
 */

#include "archi/thread/ctx/thread_group.var.h"
#include "archi/thread/api/thread_group.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <stdalign.h>

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__thread_group)
{
    // Parse parameters
    archi_thread_group_start_params_t thread_group_params = {0};
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "params", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_thread_group_start_params_t)},
            {.name = "num_threads", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            thread_group_params = *(archi_thread_group_start_params_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            thread_group_params.num_threads = *(size_t*)parsed[index].value.ptr;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_thread_group_t thread_group = archi_thread_group_create(thread_group_params, ARCHI_ERROR_PARAMETER);
    if (thread_group == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = thread_group,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__opaque_data(ARCHI_POINTER_DATA_TAG__THREAD_GROUP),
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__thread_group)
{
    archi_thread_group_destroy(context->ptr);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__thread_group)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("num_threads", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t num_threads = archi_thread_group_num_threads(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &num_threads,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__thread_group = {
    .init_fn = archi_context_init__thread_group,
    .final_fn = archi_context_final__thread_group,
    .eval_fn = archi_context_eval__thread_group,
};

