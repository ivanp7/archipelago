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
 * @brief Application context interface for HSP transition attachment data.
 */

#include "archi/hsp/ctx_hsp/attached_transition_data.var.h"
#include "archi/hsp/hsp/attached_transition.typ.h"
#include "archipelago/base/ref_count.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()
#include <stdbool.h>

struct archi_context_hsp_attached_transition_data_data {
    archi_pointer_t attached_transition;

    // References
    archi_pointer_t pre_function;
    archi_pointer_t pre_data;
    archi_pointer_t post_function;
    archi_pointer_t post_data;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_attached_transition_data_init)
{
    archi_pointer_t pre_function = {0};
    archi_pointer_t pre_data = {0};
    archi_pointer_t post_function = {0};
    archi_pointer_t post_data = {0};

    bool param_pre_function_set = false;
    bool param_pre_data_set = false;
    bool param_post_function_set = false;
    bool param_post_data_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("pre_function", params->name) == 0)
        {
            if (param_pre_function_set)
                continue;
            param_pre_function_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EVALUE;

            pre_function = params->value;
        }
        else if (strcmp("pre_data", params->name) == 0)
        {
            if (param_pre_data_set)
                continue;
            param_pre_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            pre_data = params->value;
        }
        else if (strcmp("post_function", params->name) == 0)
        {
            if (param_post_function_set)
                continue;
            param_post_function_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EVALUE;

            post_function = params->value;
        }
        else if (strcmp("post_data", params->name) == 0)
        {
            if (param_post_data_set)
                continue;
            param_post_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            post_data = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_hsp_attached_transition_data_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_hsp_attached_transition_data_t *attachment = malloc(sizeof(*attachment));
    if (attachment == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *attachment = (archi_hsp_attached_transition_data_t){
        .pre = {
            .function = (archi_hsp_transition_function_t)pre_function.fptr,
            .data = pre_data.ptr,
        },
        .post = {
            .function = (archi_hsp_transition_function_t)post_function.fptr,
            .data = post_data.ptr,
        },
    };

    *context_data = (struct archi_context_hsp_attached_transition_data_data){
        .attached_transition = {
            .ptr = attachment,
            .element = {
                .num_of = 1,
                .size = sizeof(*attachment),
                .alignment = alignof(archi_hsp_attached_transition_data_t),
            },
        },
        .pre_function = pre_function,
        .pre_data = pre_data,
        .post_function = post_function,
        .post_data = post_data,
    };

    archi_reference_count_increment(pre_function.ref_count);
    archi_reference_count_increment(pre_data.ref_count);
    archi_reference_count_increment(post_function.ref_count);
    archi_reference_count_increment(post_data.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_attached_transition_data_final)
{
    struct archi_context_hsp_attached_transition_data_data *context_data =
        (struct archi_context_hsp_attached_transition_data_data*)context;

    archi_reference_count_decrement(context_data->pre_function.ref_count);
    archi_reference_count_decrement(context_data->pre_data.ref_count);
    archi_reference_count_decrement(context_data->post_function.ref_count);
    archi_reference_count_decrement(context_data->post_data.ref_count);
    free(context_data->attached_transition.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_attached_transition_data_get)
{
    struct archi_context_hsp_attached_transition_data_data *context_data =
        (struct archi_context_hsp_attached_transition_data_data*)context;

    if (strcmp("pre.function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->pre_function;
    }
    else if (strcmp("pre.data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->pre_data;
    }
    else if (strcmp("post.function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->post_function;
    }
    else if (strcmp("post.data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->post_data;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_attached_transition_data_set)
{
    struct archi_context_hsp_attached_transition_data_data *context_data =
        (struct archi_context_hsp_attached_transition_data_data*)context;

    archi_hsp_attached_transition_data_t *attachment = context_data->attached_transition.ptr;

    if (strcmp("pre.function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->pre_function.ref_count);

        attachment->pre.function = (archi_hsp_transition_function_t)value.fptr;
        context_data->pre_function = value;
    }
    else if (strcmp("pre.data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->pre_data.ref_count);

        attachment->pre.data = value.ptr;
        context_data->pre_data = value;
    }
    else if (strcmp("post.function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->post_function.ref_count);

        attachment->post.function = (archi_hsp_transition_function_t)value.fptr;
        context_data->post_function = value;
    }
    else if (strcmp("post.data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->post_data.ref_count);

        attachment->post.data = value.ptr;
        context_data->post_data = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_hsp_attached_transition_data_interface = {
    .init_fn = archi_context_hsp_attached_transition_data_init,
    .final_fn = archi_context_hsp_attached_transition_data_final,
    .get_fn = archi_context_hsp_attached_transition_data_get,
    .set_fn = archi_context_hsp_attached_transition_data_set,
};

