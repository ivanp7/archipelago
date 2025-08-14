/**
 * @file
 * @brief Application context interfaces for OpenCL shared virtual memory.
 */

#include "archi/opencl/ctx/svm.var.h"
#include "archi/opencl/mem/svm.typ.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

#include <CL/cl.h>

struct archi_context_opencl_svm_alloc_data_data {
    archi_pointer_t alloc_data;

    // References
    archi_pointer_t context;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_svm_alloc_data_init)
{
    archi_pointer_t opencl_context = {0};
    cl_svm_mem_flags mem_flags = 0;

    bool param_context_set = false;
    bool param_mem_flags_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("context", params->name) == 0)
        {
            if (param_context_set)
                continue;
            param_context_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            opencl_context = params->value;
        }
        else if (strcmp("mem_flags", params->name) == 0)
        {
            if (param_mem_flags_set)
                continue;
            param_mem_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            mem_flags = *(cl_svm_mem_flags*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_opencl_svm_alloc_data_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_opencl_svm_alloc_data_t *alloc_data = malloc(sizeof(*alloc_data));
    if (alloc_data == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *alloc_data = (archi_opencl_svm_alloc_data_t){
        .context = opencl_context.ptr,
        .mem_flags = mem_flags,
    };

    *context_data = (struct archi_context_opencl_svm_alloc_data_data){
        .alloc_data = {
            .ptr = alloc_data,
            .element = {
                .num_of = 1,
                .size = sizeof(*alloc_data),
                .alignment = alignof(archi_opencl_svm_alloc_data_t),
            },
        },
        .context = opencl_context,
    };

    archi_reference_count_increment(opencl_context.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_svm_alloc_data_final)
{
    struct archi_context_opencl_svm_alloc_data_data *context_data =
        (struct archi_context_opencl_svm_alloc_data_data*)context;

    archi_reference_count_decrement(context_data->context.ref_count);
    free(context_data->alloc_data.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_svm_alloc_data_get)
{
    struct archi_context_opencl_svm_alloc_data_data *context_data =
        (struct archi_context_opencl_svm_alloc_data_data*)context;

    archi_opencl_svm_alloc_data_t *alloc_data = context_data->alloc_data.ptr;

    if (strcmp("context", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->context;
    }
    else if (strcmp("mem_flags", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &alloc_data->mem_flags,
            .ref_count = context_data->alloc_data.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(alloc_data->mem_flags),
                .alignment = alignof(cl_svm_mem_flags),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_opencl_svm_alloc_data_interface = {
    .init_fn = archi_context_opencl_svm_alloc_data_init,
    .final_fn = archi_context_opencl_svm_alloc_data_final,
    .get_fn = archi_context_opencl_svm_alloc_data_get,
};

///////////////////////////////////////////////////////////////////////////////

struct archi_context_opencl_svm_map_data_data {
    archi_pointer_t map_data;

    // References
    archi_pointer_t command_queue;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_svm_map_data_init)
{
    archi_pointer_t command_queue = {0};
    cl_map_flags map_flags = 0;

    bool param_command_queue_set = false;
    bool param_map_flags_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("command_queue", params->name) == 0)
        {
            if (param_command_queue_set)
                continue;
            param_command_queue_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            command_queue = params->value;
        }
        else if (strcmp("map_flags", params->name) == 0)
        {
            if (param_map_flags_set)
                continue;
            param_map_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            map_flags = *(cl_map_flags*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_opencl_svm_map_data_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_opencl_svm_map_data_t *map_data = malloc(sizeof(*map_data));
    if (map_data == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *map_data = (archi_opencl_svm_map_data_t){
        .command_queue = command_queue.ptr,
        .map_flags = map_flags,
    };

    *context_data = (struct archi_context_opencl_svm_map_data_data){
        .map_data = {
            .ptr = map_data,
            .element = {
                .num_of = 1,
                .size = sizeof(*map_data),
                .alignment = alignof(archi_opencl_svm_map_data_t),
            },
        },
        .command_queue = command_queue,
    };

    archi_reference_count_increment(command_queue.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_svm_map_data_final)
{
    struct archi_context_opencl_svm_map_data_data *context_data =
        (struct archi_context_opencl_svm_map_data_data*)context;

    archi_reference_count_decrement(context_data->command_queue.ref_count);
    free(context_data->map_data.ptr);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_svm_map_data_get)
{
    struct archi_context_opencl_svm_map_data_data *context_data =
        (struct archi_context_opencl_svm_map_data_data*)context;

    archi_opencl_svm_map_data_t *map_data = context_data->map_data.ptr;

    if (strcmp("command_queue", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->command_queue;
    }
    else if (strcmp("map_flags", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &map_data->map_flags,
            .ref_count = context_data->map_data.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(map_data->map_flags),
                .alignment = alignof(cl_map_flags),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_opencl_svm_map_data_interface = {
    .init_fn = archi_context_opencl_svm_map_data_init,
    .final_fn = archi_context_opencl_svm_map_data_final,
    .get_fn = archi_context_opencl_svm_map_data_get,
};

