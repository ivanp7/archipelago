/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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
 * @brief Application context interface for thread groups.
 */

#include "archi/res_thread/ctx/thread_group.var.h"
#include "archi/res_thread/api/thread_group.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()
#include <stdbool.h>

struct archi_context_thread_group_data {
    archi_pointer_t context;
    archi_thread_group_start_params_t params;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_thread_group_init)
{
    archi_thread_group_start_params_t thread_group_params = {0};
    archi_thread_group_start_params_t thread_group_params_fields = {0};

    bool param_params_set = false;
    bool param_num_threads_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("params", params->name) == 0)
        {
            if (param_params_set)
                continue;
            param_params_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            thread_group_params = *(archi_thread_group_start_params_t*)params->value.ptr;
        }
        else if (strcmp("num_threads", params->name) == 0)
        {
            if (param_num_threads_set)
                continue;
            param_num_threads_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            thread_group_params_fields.num_threads = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (param_num_threads_set)
        thread_group_params.num_threads = thread_group_params_fields.num_threads;

    struct archi_context_thread_group_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;
    archi_thread_group_context_t thread_group = archi_thread_group_start(thread_group_params, &code);

    if (code < 0)
    {
        free(context_data);
        return code;
    }

    *context_data = (struct archi_context_thread_group_data){
        .context = {
            .ptr = thread_group,
            .element = {
                .num_of = 1,
            },
        },
        .params = thread_group_params,
    };

    *context = (archi_pointer_t*)context_data;
    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_thread_group_final)
{
    struct archi_context_thread_group_data *context_data =
        (struct archi_context_thread_group_data*)context;

    archi_thread_group_stop(context_data->context.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_thread_group_get)
{
    struct archi_context_thread_group_data *context_data =
        (struct archi_context_thread_group_data*)context;

    if (strcmp("num_threads", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->params.num_threads,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->params.num_threads),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_thread_group_interface = {
    .init_fn = archi_context_thread_group_init,
    .final_fn = archi_context_thread_group_final,
    .get_fn = archi_context_thread_group_get,
};

