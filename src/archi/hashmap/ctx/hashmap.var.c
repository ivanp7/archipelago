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
#include "archi/hashmap/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__hashmap)
{
    // Parse parameters
    archi_hashmap_alloc_params_t hashmap_alloc_params = {0};
    {
        archi_plist_param_t parsed[] = {
            {.name = "params",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_hashmap_alloc_params_t)}},
                .assign = {archi_plist_assign__value, &hashmap_alloc_params, sizeof(hashmap_alloc_params), NULL}},
            {.name = "capacity",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &hashmap_alloc_params.capacity, sizeof(hashmap_alloc_params.capacity), NULL}},
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

    archi_hashmap_t hashmap = archi_hashmap_alloc(NULL, hashmap_alloc_params, ARCHI_ERROR_PARAM);
    if (hashmap == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = hashmap,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__HASHMAP),
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

    archi_rcpointer_t value = {0};

    ARCHI_ERROR_VAR(error);

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

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__hashmap)
{
    if (slot.num_indices != 0)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
        return;
    }

    if (!unset)
    {
        archi_hashmap_set_params_t params = {
            .insertion_allowed = true,
            .update_allowed = true,
        };

        archi_hashmap_set(context->ptr, slot.name, value, params, ARCHI_ERROR_PARAM);
    }
    else
    {
        archi_hashmap_unset_params_t params = {0};

        archi_hashmap_unset(context->ptr, slot.name, params, ARCHI_ERROR_PARAM);
    }
}

const archi_context_interface_t
archi_context_interface__hashmap = {
    .init_fn = archi_context_init__hashmap,
    .final_fn = archi_context_final__hashmap,
    .eval_fn = archi_context_eval__hashmap,
    .set_fn = archi_context_set__hashmap,
};

