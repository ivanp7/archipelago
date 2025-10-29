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
 * @brief Context interface for environment variables.
 */

#include "archi/env/ctx/variable.var.h"
#include "archi/env/api/variable.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/base/ref_count.fun.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strlen()
#include <stdbool.h>

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__env_variable)
{
    // Parse parameters
    archi_rcpointer_t default_value = {.attr = ARCHI_POINTER_ATTR__DATA_TYPE(0, char)};
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "default_value", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(0, char)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            default_value = parsed[index].value;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    default_value = archi_rcpointer_own(default_value, ARCHI_ERROR_PARAMETER);
    if (!default_value.attr) // failed to own
    {
        free(context_data);
        return NULL;
    }

    *context_data = default_value;

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__env_variable)
{
    archi_rcpointer_disown(*context);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__env_variable)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (slot.num_indices != 0)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
        return;
    }

    archi_rcpointer_t value = *context; // default value

    archi_error_t error = {0};
    char *env_var = archi_env_get(slot.name, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        return;

    if (env_var != NULL)
    {
        value = (archi_rcpointer_t){
            .ptr = env_var,
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                ARCHI_POINTER_ATTR__DATA_TYPE(strlen(env_var) + 1, char),
            .ref_count = archi_reference_count_alloc(free, env_var),
        };

        if (value.ref_count == NULL)
        {
            free(env_var);

            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate reference counter");
            return;
        }
    }

    ARCHI_CONTEXT_YIELD(value);

    if (env_var != NULL)
        archi_reference_count_decrement(value.ref_count); // abandon ownership
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__env_variable)
{
    if (ARCHI_STRING_COMPARE("default_value", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(0, char)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a string");
            return;
        }

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        *context = value;
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__env_variable = {
    .init_fn = archi_context_init__env_variable,
    .final_fn = archi_context_final__env_variable,
    .eval_fn = archi_context_eval__env_variable,
    .set_fn = archi_context_set__env_variable,
};

