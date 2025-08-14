/**
 * @file
 * @brief Application context interfaces for OpenCL devices.
 */

#include "archi/opencl/ctx/context.var.h"
#include "archi/opencl/api/device.fun.h"
#include "archipelago/log/print.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

#include <CL/cl.h>

struct archi_context_opencl_context_data {
    archi_pointer_t context;
    archi_opencl_platform_device_ids_t *ids;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_context_init)
{
    cl_uint platform_idx = 0;
    cl_uint *device_idx = NULL;
    cl_uint num_devices = 0;

    bool param_platform_idx_set = false;
    bool param_device_idx_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("platform_idx", params->name) == 0)
        {
            if (param_platform_idx_set)
                continue;
            param_platform_idx_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            platform_idx = *(cl_uint*)params->value.ptr;
        }
        else if (strcmp("device_idx", params->name) == 0)
        {
            if (param_device_idx_set)
                continue;
            param_device_idx_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            device_idx = params->value.ptr;
            num_devices = params->value.element.num_of;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archi_context_opencl_context_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;

    *context_data = (struct archi_context_opencl_context_data){
        .ids = archi_opencl_get_platform_device_ids(
                platform_idx, num_devices, device_idx, &code),
    };

    if (context_data->ids == NULL)
    {
        free(context_data);
        return code;
    }

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)context_data->ids->platform_id,
        0};

    cl_int ret;
    cl_context opencl_context = clCreateContext(properties,
            num_devices, context_data->ids->device_id, NULL, NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        archi_log_error("archi_context_opencl_context_init", "clCreateContext(<platform #%u>) failed with error %i",
                platform_idx, ret);
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    context_data->context = (archi_pointer_t){
        .ptr = opencl_context,
        .element = {
            .num_of = 1,
        },
    };

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_context_final)
{
    struct archi_context_opencl_context_data *context_data =
        (struct archi_context_opencl_context_data*)context;

    clReleaseContext(context_data->context.ptr);
    free(context_data->ids);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_context_get)
{
    struct archi_context_opencl_context_data *context_data =
        (struct archi_context_opencl_context_data*)context;

    if (strcmp("platform_id", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = context_data->ids->platform_id,
            .ref_count = context_data->context.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->ids->platform_id),
                .alignment = alignof(cl_platform_id),
            },
        };
    }
    else if (strcmp("device_id", slot.name) == 0)
    {
        if (slot.num_indices > 1)
            return ARCHI_STATUS_EMISUSE;

        if (slot.num_indices == 0)
        {
            *value = (archi_pointer_t){
                .ptr = (context_data->ids->num_devices > 0) ? context_data->ids->device_id : NULL,
                .ref_count = context_data->context.ref_count,
                .element = {
                    .num_of = context_data->ids->num_devices,
                    .size = sizeof(context_data->ids->device_id[0]),
                    .alignment = alignof(cl_device_id),
                },
            };
        }
        else
        {
            if (slot.index[0] >= context_data->ids->num_devices)
                return ARCHI_STATUS_EMISUSE;

            *value = (archi_pointer_t){
                .ptr = context_data->ids->device_id[slot.index[0]],
                .ref_count = context_data->context.ref_count,
                .element = {
                    .num_of = 1,
                    .size = sizeof(context_data->ids->device_id[0]),
                    .alignment = alignof(cl_device_id),
                },
            };
        }
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_opencl_context_interface = {
    .init_fn = archi_context_opencl_context_init,
    .final_fn = archi_context_opencl_context_final,
    .get_fn = archi_context_opencl_context_get,
};

