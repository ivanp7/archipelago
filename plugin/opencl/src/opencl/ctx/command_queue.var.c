/**
 * @file
 * @brief Application context interfaces for OpenCL command queues.
 */

#include "archip/opencl/ctx/command_queue.var.h"
#include "archi/log/print.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()

#include <CL/cl.h>

struct archip_context_opencl_command_queue_data {
    archi_pointer_t command_queue;
    archi_pointer_t context;
    archi_pointer_t device_id;
};

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_command_queue_init)
{
    archi_pointer_t opencl_context = {0};
    archi_pointer_t device_id = {0};
    bool out_of_order_exec = false;
    bool profiling = false;
#if defined(cl_khr_priority_hints)
    cl_queue_priority_khr priority_hint = 0;
#endif
#if defined(cl_khr_throttle_hints)
    cl_queue_throttle_khr throttle_hint = 0;
#endif

    bool param_context_set = false;
    bool param_device_id_set = false;
    bool param_out_of_order_exec_set = false;
    bool param_profiling_set = false;
#if defined(cl_khr_priority_hints)
    bool param_priority_hint_set = false;
#endif
#if defined(cl_khr_throttle_hints)
    bool param_throttle_hint_set = false;
#endif

    for (; params != NULL; params = params->next)
    {
        if (strcmp("context", params->name) == 0)
        {
            if (param_context_set)
                continue;
            param_context_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            opencl_context = params->value;
        }
        else if (strcmp("device_id", params->name) == 0)
        {
            if (param_device_id_set)
                continue;
            param_device_id_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            device_id = params->value;
        }
        else if (strcmp("out_of_order_exec", params->name) == 0)
        {
            if (param_out_of_order_exec_set)
                continue;
            param_out_of_order_exec_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            out_of_order_exec = *(char*)params->value.ptr;
        }
        else if (strcmp("profiling", params->name) == 0)
        {
            if (param_profiling_set)
                continue;
            param_profiling_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            profiling = *(char*)params->value.ptr;
        }
#if defined(cl_khr_priority_hints)
        else if (strcmp("priority_hint", params->name) == 0)
        {
            if (param_priority_hint_set)
                continue;
            param_priority_hint_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            priority_hint = *(cl_queue_priority_khr*)params->value.ptr;
        }
#endif
#if defined(cl_khr_throttle_hints)
        else if (strcmp("throttle_hint", params->name) == 0)
        {
            if (param_throttle_hint_set)
                continue;
            param_throttle_hint_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            throttle_hint = *(cl_queue_throttle_khr*)params->value.ptr;
        }
#endif
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archip_context_opencl_command_queue_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    cl_queue_properties properties[] = {
        CL_QUEUE_PROPERTIES, (out_of_order_exec ? CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE : 0) |
            (profiling ? CL_QUEUE_PROFILING_ENABLE : 0),
#if defined(cl_khr_priority_hints)
        CL_QUEUE_PRIORITY_KHR, priority_hint,
#endif
#if defined(cl_khr_throttle_hints)
        CL_QUEUE_THROTTLE_KHR, throttle_hint,
#endif
        0};

    cl_int ret;
    cl_command_queue command_queue = clCreateCommandQueueWithProperties(
            opencl_context.ptr, device_id.ptr, properties, &ret);

    if (ret != CL_SUCCESS)
    {
        archi_log_error("archip_context_opencl_command_queue_init", "clCreateCommandQueueWithProperties() failed with error %i", ret);
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    *context_data = (struct archip_context_opencl_command_queue_data){
        .command_queue = {
            .ptr = command_queue,
            .element = {
                .num_of = 1,
            },
        },
        .context = opencl_context,
        .device_id = device_id,
    };

    archi_reference_count_increment(opencl_context.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_command_queue_final)
{
    struct archip_context_opencl_command_queue_data *context_data =
        (struct archip_context_opencl_command_queue_data*)context;

    clReleaseCommandQueue(context_data->command_queue.ptr);
    archi_reference_count_decrement(context_data->context.ref_count);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_command_queue_get)
{
    struct archip_context_opencl_command_queue_data *context_data =
        (struct archip_context_opencl_command_queue_data*)context;

    if (strcmp("context", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->context;
    }
    else if (strcmp("device_id", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->device_id;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archip_context_opencl_command_queue_interface = {
    .init_fn = archip_context_opencl_command_queue_init,
    .final_fn = archip_context_opencl_command_queue_final,
    .get_fn = archip_context_opencl_command_queue_get,
};

