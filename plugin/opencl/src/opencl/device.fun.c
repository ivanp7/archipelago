/**
 * @file
 * @brief Operations on OpenCL devices.
 */

#include "archip/opencl/device.fun.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), free()

#include <CL/cl.h>

archip_opencl_platform_device_ids_t*
archip_opencl_get_platform_device_ids(
        cl_uint platform_index,

        cl_uint num_device_indices,
        const cl_uint *device_index,

        archi_status_t *code)
{
    if ((num_device_indices > 0) && (device_index == NULL))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    archip_opencl_platform_device_ids_t *ids = malloc(ARCHI_SIZEOF_FLEXIBLE(
                archip_opencl_platform_device_ids_t, device_id, num_device_indices));
    if (ids == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return NULL;
    }

    // Obtain platform ID
    {
        cl_uint num_platforms;

        cl_int ret = clGetPlatformIDs(0, (cl_platform_id*)NULL, &num_platforms);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            free(ids);
            return NULL;
        }

        if (platform_index >= num_platforms)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_EMISUSE;

            free(ids);
            return NULL;
        }

        cl_platform_id *platform_list = malloc(sizeof(cl_platform_id) * num_platforms);
        if (platform_list == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            free(ids);
            return NULL;
        }

        ret = clGetPlatformIDs(num_platforms, platform_list, (cl_uint*)NULL);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            free(platform_list);
            free(ids);
            return NULL;
        }

        ids->platform_id = platform_list[platform_index];

        free(platform_list);
    }

    // Obtain device IDs
    if (num_device_indices > 0)
    {
        cl_uint num_devices;

        cl_int ret = clGetDeviceIDs(ids->platform_id, CL_DEVICE_TYPE_ALL,
                0, (cl_device_id*)NULL, &num_devices);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            free(ids);
            return NULL;
        }

        cl_device_id *device_list = malloc(sizeof(cl_device_id) * num_devices);
        if (device_list == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            free(ids);
            return NULL;
        }

        for (cl_uint i = 0; i < num_device_indices; i++)
            if (device_index[i] >= num_devices)
            {
                if (code != NULL)
                    *code = ARCHI_STATUS_EMISUSE;

                free(ids);
                return NULL;
            }

        ret = clGetDeviceIDs(ids->platform_id, CL_DEVICE_TYPE_ALL,
                num_devices, device_list, (cl_uint*)NULL);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            free(device_list);
            free(ids);
            return NULL;
        }

        ids->num_devices = num_devices;
        for (cl_uint i = 0; i < num_device_indices; i++)
            ids->device_id[i] = device_list[device_index[i]];

        free(device_list);
    }
    else
        ids->num_devices = 0;

    if (code != NULL)
        *code = 0;

    return ids;
}

