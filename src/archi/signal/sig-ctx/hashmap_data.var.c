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
 * @brief Context interface for data of signal meta-handler for multiple handler support.
 */

#include "archi/signal/sig-ctx/hashmap_data.var.h"
#include "archi/signal/sig/hashmap.typ.h"
#include "archi/signal/api/handler.typ.h"
#include "archi/hashmap/api/hashmap.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__signal_handler_data__hashmap)
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

    archi_signal_handler_data__hashmap_t *handler_data = malloc(sizeof(*handler_data));
    if (handler_data == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate signal handler data");
        return NULL;
    }

    handler_data->hashmap = archi_hashmap_alloc(hashmap_alloc_params, ARCHI_ERROR_PARAMETER);
    if (handler_data->hashmap == NULL)
    {
        free(handler_data);
        free(context_data);

        return NULL;
    }

    {
        int ret = mtx_init(&handler_data->hashmap_lock, mtx_plain);
        if (ret != thrd_success)
        {
            archi_hashmap_free(handler_data->hashmap);
            free(handler_data);
            free(context_data);

            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't initialize hashmap guard mutex");
            return NULL;
        }
    }

    *context_data = (archi_rcpointer_t){
        .ptr = handler_data,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_signal_handler_data__hashmap_t),
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__signal_handler_data__hashmap)
{
    archi_signal_handler_data__hashmap_t *handler_data = context->ptr;

    mtx_destroy(&handler_data->hashmap_lock);
    archi_hashmap_free(handler_data->hashmap);
    free(handler_data);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__signal_handler_data__hashmap)
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

    archi_signal_handler_data__hashmap_t *handler_data = context->ptr;

    {
        int ret = mtx_lock(&handler_data->hashmap_lock);
        if (ret != thrd_success)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't lock hashmap mutex");
            return;
        }
    }

    archi_rcpointer_t value = {0};

    archi_error_t error = {0};
    bool success = archi_hashmap_get(handler_data->hashmap, slot.name, &value, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (!success)
    {
        if (error.code == 0)
            ARCHI_ERROR_SET(ARCHI__EKEY, "signal handler '%s' not found", slot.name);

        mtx_unlock(&handler_data->hashmap_lock);
        return;
    }

    mtx_unlock(&handler_data->hashmap_lock);

    ARCHI_CONTEXT_YIELD(value);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__signal_handler_data__hashmap)
{
    if (slot.num_indices != 0)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
        return;
    }
    else if (!archi_pointer_attr_compatible(value.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(value.ptr != NULL, archi_signal_handler_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a signal handler");
        return;
    }

    archi_signal_handler_data__hashmap_t *handler_data = context->ptr;

    {
        int ret = mtx_lock(&handler_data->hashmap_lock);
        if (ret != thrd_success)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't lock hashmap mutex");
            return;
        }
    }

    if (value.ptr != NULL) // set handler
    {
        archi_hashmap_set_params_t params = {
            .insertion_allowed = true,
            .update_allowed = true,
        };
        archi_hashmap_set(handler_data->hashmap, slot.name, value, params, ARCHI_ERROR_PARAMETER);
    }
    else // unset handler
    {
        archi_hashmap_unset_params_t params = {0};
        archi_hashmap_unset(handler_data->hashmap, slot.name, params, ARCHI_ERROR_PARAMETER);
    }

    mtx_unlock(&handler_data->hashmap_lock);
}

const archi_context_interface_t
archi_context_interface__signal_handler_data__hashmap = {
    .init_fn = archi_context_init__signal_handler_data__hashmap,
    .final_fn = archi_context_final__signal_handler_data__hashmap,
    .eval_fn = archi_context_eval__signal_handler_data__hashmap,
    .set_fn = archi_context_set__signal_handler_data__hashmap,
};

