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

#include "archi/builtin/res_thread_group/context.var.h"
#include "archi/builtin/res_thread_group/dispatch.typ.h"
#include "archi/res/thread_group/api.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_init)
{
    archi_thread_group_start_params_t thread_group_params = {0};

    bool param_struct_set = false;
    bool param_num_threads_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("params", params->name) == 0)
        {
            if (param_struct_set)
                continue;
            param_struct_set = true;

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

            thread_group_params.num_threads = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    size_t *num_threads = malloc(sizeof(*num_threads));
    if (num_threads == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *num_threads = thread_group_params.num_threads;

    archi_status_t code;
    archi_thread_group_context_t thread_group =
        archi_thread_group_start(thread_group_params, &code);

    if (code < 0)
    {
        free(num_threads);
        return code;
    }

    context->public_value = (archi_pointer_t){
        .ptr = thread_group,
        .element = {
            .num_of = 1,
        },
    };

    context->private_value = (archi_pointer_t){
        .ptr = num_threads,
        .element = {
            .num_of = 1,
            .size = sizeof(*num_threads),
            .alignment = alignof(size_t),
        },
    };

    return code;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_final)
{
    archi_thread_group_stop(context.public_value.ptr);
    free(context.private_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_get)
{
    if (strcmp("num_threads", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = context.private_value.ptr,
            .ref_count = context.public_value.ref_count,
            .element = context.private_value.element,
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_res_thread_group_interface = {
    .init_fn = archi_context_res_thread_group_init,
    .final_fn = archi_context_res_thread_group_final,
    .get_fn = archi_context_res_thread_group_get,
};

/*****************************************************************************/

#define REF_FUNCTION 0
#define REF_DATA 1
#define NUM_REFERENCES 2

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_work_init)
{
    archi_pointer_t work_function = {0};
    archi_pointer_t work_data = {0};
    size_t work_size = 0;

    bool param_function_set = false;
    bool param_data_set = false;
    bool param_size_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("function", params->name) == 0)
        {
            if (param_function_set)
                continue;
            param_function_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EVALUE;

            work_function = params->value;
        }
        else if (strcmp("data", params->name) == 0)
        {
            if (param_data_set)
                continue;
            param_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            work_data = params->value;
        }
        else if (strcmp("size", params->name) == 0)
        {
            if (param_size_set)
                continue;
            param_size_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            work_size = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_thread_group_work_t *work = malloc(sizeof(*work));
    if (work == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *work = (archi_thread_group_work_t){
        .function = (archi_thread_group_work_func_t)work_function.fptr,
        .data = work_data.ptr,
        .size = work_size,
    };

    context->num_references = NUM_REFERENCES;
    context->reference = malloc(sizeof(*context->reference) * context->num_references);
    if (context->reference == NULL)
    {
        free(work);
        return ARCHI_STATUS_ENOMEMORY;
    }

    context->reference[REF_FUNCTION] = work_function;
    context->reference[REF_DATA] = work_data;

    for (size_t i = 0; i < NUM_REFERENCES; i++)
        archi_reference_count_increment(context->reference[i].ref_count);

    context->public_value = (archi_pointer_t){
        .ptr = work,
        .element = {
            .num_of = 1,
            .size = sizeof(*work),
            .alignment = alignof(archi_thread_group_work_t),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_work_final)
{
    for (size_t i = context.num_references; i-- > 0;)
        archi_reference_count_decrement(context.reference[i].ref_count);

    free(context.reference);
    free(context.public_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_work_get)
{
    archi_thread_group_work_t *work = context.public_value.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_FUNCTION];
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_DATA];
    }
    else if (strcmp("size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &work->size,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(work->size),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_res_thread_group_work_set)
{
    archi_thread_group_work_t *work = context.public_value.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_FUNCTION].ref_count);

        work->function = (archi_thread_group_work_func_t)value.fptr;
        context.reference[REF_FUNCTION] = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_DATA].ref_count);

        work->data = value.ptr;
        context.reference[REF_DATA] = value;
    }
    else if (strcmp("size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        work->size = *(size_t*)value.ptr;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_res_thread_group_work_interface = {
    .init_fn = archi_context_res_thread_group_work_init,
    .final_fn = archi_context_res_thread_group_work_final,
    .get_fn = archi_context_res_thread_group_work_get,
    .set_fn = archi_context_res_thread_group_work_set,
};

#undef REF_FUNCTION
#undef REF_DATA
#undef NUM_REFERENCES

/*****************************************************************************/

#define REF_FUNCTION 0
#define REF_DATA 1
#define NUM_REFERENCES 2

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_callback_init)
{
    archi_pointer_t callback_function = {0};
    archi_pointer_t callback_data = {0};

    bool param_function_set = false;
    bool param_data_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("function", params->name) == 0)
        {
            if (param_function_set)
                continue;
            param_function_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
                return ARCHI_STATUS_EVALUE;

            callback_function = params->value;
        }
        else if (strcmp("data", params->name) == 0)
        {
            if (param_data_set)
                continue;
            param_data_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            callback_data = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_thread_group_callback_t *callback = malloc(sizeof(*callback));
    if (callback == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *callback = (archi_thread_group_callback_t){
        .function = (archi_thread_group_callback_func_t)callback_function.fptr,
        .data = callback_data.ptr,
    };

    context->num_references = NUM_REFERENCES;
    context->reference = malloc(sizeof(*context->reference) * context->num_references);
    if (context->reference == NULL)
    {
        free(callback);
        return ARCHI_STATUS_ENOMEMORY;
    }

    context->reference[REF_FUNCTION] = callback_function;
    context->reference[REF_DATA] = callback_data;

    for (size_t i = 0; i < NUM_REFERENCES; i++)
        archi_reference_count_increment(context->reference[i].ref_count);

    context->public_value = (archi_pointer_t){
        .ptr = callback,
        .element = {
            .num_of = 1,
            .size = sizeof(*callback),
            .alignment = alignof(archi_thread_group_callback_t),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_callback_final)
{
    for (size_t i = context.num_references; i-- > 0;)
        archi_reference_count_decrement(context.reference[i].ref_count);

    free(context.reference);
    free(context.public_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_callback_get)
{
    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_FUNCTION];
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_DATA];
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_res_thread_group_callback_set)
{
    archi_thread_group_callback_t *callback = context.public_value.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_FUNCTION].ref_count);

        callback->function = (archi_thread_group_callback_func_t)value.fptr;
        context.reference[REF_FUNCTION] = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_DATA].ref_count);

        callback->data = value.ptr;
        context.reference[REF_DATA] = value;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_res_thread_group_callback_interface = {
    .init_fn = archi_context_res_thread_group_callback_init,
    .final_fn = archi_context_res_thread_group_callback_final,
    .get_fn = archi_context_res_thread_group_callback_get,
    .set_fn = archi_context_res_thread_group_callback_set,
};

#undef REF_FUNCTION
#undef REF_DATA
#undef NUM_REFERENCES

/*****************************************************************************/

#define REF_CONTEXT 0
#define REF_WORK 1
#define REF_CALLBACK 2
#define NUM_REFERENCES 3

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_dispatch_data_init)
{
    archi_pointer_t thread_group_context = {0};
    archi_pointer_t thread_group_work = {0};
    archi_pointer_t thread_group_callback = {0};
    archi_thread_group_dispatch_params_t thread_group_dispatch_params = {0};

    bool param_context_set = false;
    bool param_work_set = false;
    bool param_callback_set = false;
    bool param_dispatch_params_set = false;
    bool param_dispatch_batch_size_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("context", params->name) == 0)
        {
            if (param_context_set)
                continue;
            param_context_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            thread_group_context = params->value;
        }
        else if (strcmp("work", params->name) == 0)
        {
            if (param_work_set)
                continue;
            param_work_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            thread_group_work = params->value;
        }
        else if (strcmp("callback", params->name) == 0)
        {
            if (param_callback_set)
                continue;
            param_callback_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            thread_group_callback = params->value;
        }
        else if (strcmp("params", params->name) == 0)
        {
            if (param_dispatch_params_set)
                continue;
            param_dispatch_params_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            thread_group_dispatch_params = *(archi_thread_group_dispatch_params_t*)params->value.ptr;
        }
        else if (strcmp("batch_size", params->name) == 0)
        {
            if (param_dispatch_batch_size_set)
                continue;
            param_dispatch_batch_size_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            thread_group_dispatch_params.batch_size = *(size_t*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_context_res_thread_group_dispatch_data_t *dispatch_data = malloc(sizeof(*dispatch_data));
    if (dispatch_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *dispatch_data = (archi_context_res_thread_group_dispatch_data_t){
        .context = thread_group_context.ptr,
        .params = thread_group_dispatch_params,
    };

    if (thread_group_work.ptr != NULL)
        dispatch_data->work = *(archi_thread_group_work_t*)thread_group_work.ptr;

    if (thread_group_callback.ptr != NULL)
        dispatch_data->callback = *(archi_thread_group_callback_t*)thread_group_callback.ptr;

    context->num_references = NUM_REFERENCES;
    context->reference = malloc(sizeof(*context->reference) * context->num_references);
    if (context->reference == NULL)
    {
        free(dispatch_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    context->reference[REF_CONTEXT] = thread_group_context;
    context->reference[REF_WORK] = thread_group_work;
    context->reference[REF_CALLBACK] = thread_group_callback;

    for (size_t i = 0; i < NUM_REFERENCES; i++)
        archi_reference_count_increment(context->reference[i].ref_count);

    context->public_value = (archi_pointer_t){
        .ptr = dispatch_data,
        .element = {
            .num_of = 1,
            .size = sizeof(*dispatch_data),
            .alignment = alignof(archi_context_res_thread_group_dispatch_data_t),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_dispatch_data_final)
{
    for (size_t i = context.num_references; i-- > 0;)
        archi_reference_count_decrement(context.reference[i].ref_count);

    free(context.reference);
    free(context.public_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_dispatch_data_get)
{
    archi_context_res_thread_group_dispatch_data_t *dispatch_data = context.public_value.ptr;

    if (strcmp("context", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_CONTEXT];
    }
    else if (strcmp("work", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_WORK];
    }
    else if (strcmp("callback", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context.reference[REF_CALLBACK];
    }
    else if (strcmp("batch_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &dispatch_data->params.batch_size,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(dispatch_data->params.batch_size),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_res_thread_group_dispatch_data_set)
{
    archi_context_res_thread_group_dispatch_data_t *dispatch_data = context.public_value.ptr;

    if (strcmp("context", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_CONTEXT].ref_count);

        dispatch_data->context = value.ptr;
        context.reference[REF_CONTEXT] = value;
    }
    else if (strcmp("work", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_WORK].ref_count);

        if (value.ptr != NULL)
            dispatch_data->work = *(archi_thread_group_work_t*)value.ptr;
        else
            dispatch_data->work = (archi_thread_group_work_t){0};

        context.reference[REF_WORK] = value;
    }
    else if (strcmp("callback", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context.reference[REF_CALLBACK].ref_count);

        if (value.ptr != NULL)
            dispatch_data->callback = *(archi_thread_group_callback_t*)value.ptr;
        else
            dispatch_data->callback = (archi_thread_group_callback_t){0};

        context.reference[REF_CALLBACK] = value;
    }
    else if (strcmp("batch_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        dispatch_data->params.batch_size = *(size_t*)value.ptr;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_res_thread_group_dispatch_data_interface = {
    .init_fn = archi_context_res_thread_group_dispatch_data_init,
    .final_fn = archi_context_res_thread_group_dispatch_data_final,
    .get_fn = archi_context_res_thread_group_dispatch_data_get,
    .set_fn = archi_context_res_thread_group_dispatch_data_set,
};

#undef REF_CONTEXT
#undef REF_WORK
#undef REF_CALLBACK
#undef NUM_REFERENCES

