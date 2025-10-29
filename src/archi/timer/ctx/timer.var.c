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
 * @brief Context interface for timers.
 */

#include "archi/timer/ctx/timer.var.h"
#include "archi/timer/api/timer.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__timer)
{
    // Parse parameters
    const char *name = NULL;
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "name", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, char)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            name = parsed[index].value.ptr;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_timer_t timer = archi_timer_alloc(name);
    if (timer == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate timer");
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = timer,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__opaque_data(ARCHI_POINTER_DATA_TAG__TIMER),
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__timer)
{
    archi_timer_free(context->ptr);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__timer)
{
    (void) ARCHI_CONTEXT_CALLBACK_PARAMETER;

    if (!call)
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "no getter slots are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("reset", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (params != NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "no parameters are accepted");
            return;
        }

        archi_timer_reset(context->ptr);
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__timer = {
    .init_fn = archi_context_init__timer,
    .final_fn = archi_context_final__timer,
    .eval_fn = archi_context_eval__timer,
};

