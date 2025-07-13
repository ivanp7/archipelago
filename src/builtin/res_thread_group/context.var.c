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
#include <string.h> // for strcmp(), strlen(), memcpy()
#include <stdalign.h> // for alignof()

struct archi_context_res_thread_group_data {
    archi_pointer_t context;
    archi_thread_group_start_params_t params;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_init)
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

    struct archi_context_res_thread_group_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;
    archi_thread_group_context_t thread_group = archi_thread_group_start(thread_group_params, &code);

    if (code < 0)
    {
        free(context_data);
        return code;
    }

    *context_data = (struct archi_context_res_thread_group_data){
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

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_final)
{
    struct archi_context_res_thread_group_data *context_data =
        (struct archi_context_res_thread_group_data*)context;

    archi_thread_group_stop(context_data->context.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_get)
{
    struct archi_context_res_thread_group_data *context_data =
        (struct archi_context_res_thread_group_data*)context;

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

const archi_context_interface_t archi_context_res_thread_group_interface = {
    .init_fn = archi_context_res_thread_group_init,
    .final_fn = archi_context_res_thread_group_final,
    .get_fn = archi_context_res_thread_group_get,
};

/*****************************************************************************/

struct archi_context_res_thread_group_work_data {
    archi_pointer_t work;

    // References
    archi_pointer_t work_function;
    archi_pointer_t work_data;
};

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

    struct archi_context_res_thread_group_work_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_thread_group_work_t *work = malloc(sizeof(*work));
    if (work == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *work = (archi_thread_group_work_t){
        .function = (archi_thread_group_work_func_t)work_function.fptr,
        .data = work_data.ptr,
        .size = work_size,
    };

    *context_data = (struct archi_context_res_thread_group_work_data){
        .work = {
            .ptr = work,
            .element = {
                .num_of = 1,
                .size = sizeof(*work),
                .alignment = alignof(archi_thread_group_work_t),
            },
        },
        .work_function = work_function,
        .work_data = work_data,
    };

    archi_reference_count_increment(work_function.ref_count);
    archi_reference_count_increment(work_data.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_work_final)
{
    struct archi_context_res_thread_group_work_data *context_data =
        (struct archi_context_res_thread_group_work_data*)context;

    archi_reference_count_decrement(context_data->work_function.ref_count);
    archi_reference_count_decrement(context_data->work_data.ref_count);
    free(context_data->work.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_work_get)
{
    struct archi_context_res_thread_group_work_data *context_data =
        (struct archi_context_res_thread_group_work_data*)context;

    archi_thread_group_work_t *work = context_data->work.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->work_function;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->work_data;
    }
    else if (strcmp("size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &work->size,
            .ref_count = context->ref_count,
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
    struct archi_context_res_thread_group_work_data *context_data =
        (struct archi_context_res_thread_group_work_data*)context;

    archi_thread_group_work_t *work = context_data->work.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->work_function.ref_count);

        work->function = (archi_thread_group_work_func_t)value.fptr;
        context_data->work_function = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->work_data.ref_count);

        work->data = value.ptr;
        context_data->work_data = value;
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

/*****************************************************************************/

struct archi_context_res_thread_group_callback_data {
    archi_pointer_t callback;

    // References
    archi_pointer_t callback_function;
    archi_pointer_t callback_data;
};

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

    struct archi_context_res_thread_group_callback_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_thread_group_callback_t *callback = malloc(sizeof(*callback));
    if (callback == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *callback = (archi_thread_group_callback_t){
        .function = (archi_thread_group_callback_func_t)callback_function.fptr,
        .data = callback_data.ptr,
    };

    *context_data = (struct archi_context_res_thread_group_callback_data){
        .callback = {
            .ptr = callback,
            .element = {
                .num_of = 1,
                .size = sizeof(*callback),
                .alignment = alignof(archi_thread_group_callback_t),
            },
        },
        .callback_function = callback_function,
        .callback_data = callback_data,
    };

    archi_reference_count_increment(callback_function.ref_count);
    archi_reference_count_increment(callback_data.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_callback_final)
{
    struct archi_context_res_thread_group_callback_data *context_data =
        (struct archi_context_res_thread_group_callback_data*)context;

    archi_reference_count_decrement(context_data->callback_function.ref_count);
    archi_reference_count_decrement(context_data->callback_data.ref_count);
    free(context_data->callback.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_callback_get)
{
    struct archi_context_res_thread_group_callback_data *context_data =
        (struct archi_context_res_thread_group_callback_data*)context;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->callback_function;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->callback_data;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_res_thread_group_callback_set)
{
    struct archi_context_res_thread_group_callback_data *context_data =
        (struct archi_context_res_thread_group_callback_data*)context;

    archi_thread_group_callback_t *callback = context_data->callback.ptr;

    if (strcmp("function", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->callback_function.ref_count);

        callback->function = (archi_thread_group_callback_func_t)value.fptr;
        context_data->callback_function = value;
    }
    else if (strcmp("data", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->callback_data.ref_count);

        callback->data = value.ptr;
        context_data->callback_data = value;
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

/*****************************************************************************/

struct archi_context_res_thread_group_dispatch_data_data {
    archi_pointer_t dispatch_data;

    // References
    archi_pointer_t context;
    archi_pointer_t work;
    archi_pointer_t callback;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_dispatch_data_init)
{
    archi_pointer_t thread_group_context = {0};
    archi_pointer_t thread_group_work = {0};
    archi_pointer_t thread_group_callback = {0};
    archi_thread_group_dispatch_params_t thread_group_dispatch_params = {0};
    archi_thread_group_dispatch_params_t thread_group_dispatch_params_fields = {0};
    char *name = NULL;

    bool param_context_set = false;
    bool param_work_set = false;
    bool param_callback_set = false;
    bool param_dispatch_params_set = false;
    bool param_dispatch_batch_size_set = false;
    bool param_name_set = false;

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

            thread_group_dispatch_params_fields.batch_size = *(size_t*)params->value.ptr;
        }
        else if (strcmp("name", params->name) == 0)
        {
            if (param_name_set)
                continue;
            param_name_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            name = params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (param_dispatch_batch_size_set)
        thread_group_dispatch_params.batch_size = thread_group_dispatch_params_fields.batch_size;

    struct archi_context_res_thread_group_dispatch_data_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_context_res_thread_group_dispatch_data_t *dispatch_data = malloc(sizeof(*dispatch_data));
    if (dispatch_data == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    if (name != NULL)
    {
        size_t name_len = strlen(name) + 1;
        char *name_copy = malloc(name_len);
        if (name_copy == NULL)
        {
            free(dispatch_data);
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        memcpy(name_copy, name, name_len);
        name = name_copy;
    }

    *dispatch_data = (archi_context_res_thread_group_dispatch_data_t){
        .context = thread_group_context.ptr,
        .work = thread_group_work.ptr,
        .callback = thread_group_callback.ptr,
        .params = thread_group_dispatch_params,
        .name = name,
    };

    *context_data = (struct archi_context_res_thread_group_dispatch_data_data){
        .dispatch_data = {
            .ptr = dispatch_data,
            .element = {
                .num_of = 1,
                .size = sizeof(*dispatch_data),
                .alignment = alignof(archi_context_res_thread_group_dispatch_data_t),
            },
        },
        .context = thread_group_context,
        .work = thread_group_work,
        .callback = thread_group_callback,
    };

    archi_reference_count_increment(thread_group_context.ref_count);
    archi_reference_count_increment(thread_group_work.ref_count);
    archi_reference_count_increment(thread_group_callback.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_dispatch_data_final)
{
    struct archi_context_res_thread_group_dispatch_data_data *context_data =
        (struct archi_context_res_thread_group_dispatch_data_data*)context;

    archi_context_res_thread_group_dispatch_data_t *dispatch_data = context_data->dispatch_data.ptr;

    archi_reference_count_decrement(context_data->context.ref_count);
    archi_reference_count_decrement(context_data->work.ref_count);
    archi_reference_count_decrement(context_data->callback.ref_count);
    free((char*)dispatch_data->name);
    free(dispatch_data);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_dispatch_data_get)
{
    struct archi_context_res_thread_group_dispatch_data_data *context_data =
        (struct archi_context_res_thread_group_dispatch_data_data*)context;

    archi_context_res_thread_group_dispatch_data_t *dispatch_data = context_data->dispatch_data.ptr;

    if (strcmp("context", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->context;
    }
    else if (strcmp("work", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->work;
    }
    else if (strcmp("callback", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->callback;
    }
    else if (strcmp("batch_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &dispatch_data->params.batch_size,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(dispatch_data->params.batch_size),
                .alignment = alignof(size_t),
            },
        };
    }
    else if (strcmp("name", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        if (dispatch_data->name != NULL)
            *value = (archi_pointer_t){
                .ptr = (void*)dispatch_data->name,
                .ref_count = context->ref_count,
                .element = {
                    .num_of = strlen(dispatch_data->name) + 1,
                    .size = 1,
                    .alignment = 1,
                },
            };
        else
            *value = (archi_pointer_t){0};
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_res_thread_group_dispatch_data_set)
{
    struct archi_context_res_thread_group_dispatch_data_data *context_data =
        (struct archi_context_res_thread_group_dispatch_data_data*)context;

    archi_context_res_thread_group_dispatch_data_t *dispatch_data = context_data->dispatch_data.ptr;

    if (strcmp("context", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->context.ref_count);

        dispatch_data->context = value.ptr;
        context_data->context = value;
    }
    else if (strcmp("work", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->work.ref_count);

        dispatch_data->work = value.ptr;
        context_data->work = value;
    }
    else if (strcmp("callback", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->callback.ref_count);

        dispatch_data->callback = value.ptr;
        context_data->callback = value;
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

