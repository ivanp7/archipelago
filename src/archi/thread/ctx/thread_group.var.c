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
#include "archi/thread/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <stdalign.h>


static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__thread_group)
{
    // Parse parameters
    archi_thread_group_start_params_t thread_group_params = {0};
    {
        archi_plist_param_t parsed[] = {
            {.name = "params",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_thread_group_start_params_t)}},
                .assign = {archi_plist_assign__value, &thread_group_params, sizeof(thread_group_params)}},
            {.name = "num_threads",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &thread_group_params.num_threads, sizeof(thread_group_params.num_threads)}},
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

    archi_thread_group_t thread_group = archi_thread_group_create(thread_group_params, ARCHI_ERROR_PARAM);
    if (thread_group == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = thread_group,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__THREAD_GROUP),
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
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
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

