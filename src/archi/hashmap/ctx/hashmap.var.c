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
 * @brief Context interface for hashmaps.
 */

#include "archi/hashmap/ctx/hashmap.var.h"
#include "archi/hashmap/api/hashmap.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__hashmap)
{
    // Parse parameters
    archi_hashmap_alloc_params_t hashmap_alloc_params = {0};
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "params", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_hashmap_alloc_params_t)},
            {.name = "capacity", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            hashmap_alloc_params = *(archi_hashmap_alloc_params_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            hashmap_alloc_params.capacity = *(size_t*)parsed[index].value.ptr;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_hashmap_t hashmap = archi_hashmap_alloc(hashmap_alloc_params, ARCHI_ERROR_PARAMETER);
    if (hashmap == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = hashmap,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__opaque_data(ARCHI_POINTER_DATA_TAG__HASHMAP),
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__hashmap)
{
    archi_hashmap_free(context->ptr);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__hashmap)
{
    if (!call)
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_rcpointer_t value = {0};

        archi_error_t error = {0};
        bool success = archi_hashmap_get(context->ptr, slot.name, &value, &error);
        ARCHI_ERROR_ASSIGN(error);

        if (!success)
        {
            if (error.code == 0)
                ARCHI_ERROR_SET(ARCHI__EKEY, "key '%s' not found", slot.name);
            return;
        }

        ARCHI_CONTEXT_YIELD(value);
    }
    else
    {
        if (ARCHI_STRING_COMPARE("", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            // Parse parameters
            const char *key = NULL;
            bool op_unset = false;
            {
                archi_kvlist_parameter_t parsed[] = {
                    {.name = "unset", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, char)},
                };

                if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                            ARCHI_ERROR_PARAMETER))
                    return;

                size_t index = 0;
                op_unset = parsed[index].value_set;
                if (op_unset)
                    key = parsed[index].value.ptr;
            }

            // Perform a hashmap operation
            if (op_unset)
            {
                archi_hashmap_unset_params_t params = {0};

                archi_error_t error = {0};
                bool success = archi_hashmap_unset(context->ptr, key, params, &error);
                ARCHI_ERROR_ASSIGN(error);

                if (!success)
                {
                    if (error.code == 0)
                        ARCHI_ERROR_SET(ARCHI__EKEY, "key '%s' not found", key);
                }

                ARCHI_ERROR_RESET();
                return;
            }
        }
        else
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
            return;
        }
    }
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__hashmap)
{
    if (slot.num_indices != 0)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
        return;
    }

    archi_hashmap_set_params_t params = {
        .insertion_allowed = true,
        .update_allowed = true,
    };

    archi_hashmap_set(context->ptr, slot.name, value, params, ARCHI_ERROR_PARAMETER);
}

const archi_context_interface_t
archi_context_interface__hashmap = {
    .init_fn = archi_context_init__hashmap,
    .final_fn = archi_context_final__hashmap,
    .eval_fn = archi_context_eval__hashmap,
    .set_fn = archi_context_set__hashmap,
};

