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
 * @brief Operations on OpenCL devices.
 */

#include "archi/opencl/api/device.fun.h"
#include "archi_base/util/size.def.h"

#include <stdlib.h> // for malloc(), free()

#include <CL/cl.h>


archi_opencl_platdev_t*
archi_opencl_platdev_alloc(
        cl_uint num_devices)
{
    archi_opencl_platdev_t *platdev = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archi_opencl_platdev_t, device_id, num_devices));
    if (platdev == NULL)
        return NULL;

    platdev->platform_id = NULL;
    {
        cl_uint *num_devices_ptr = (cl_uint*)&platdev->num_devices;
        *num_devices_ptr = num_devices;
    }
    for (cl_uint i = 0; i < num_devices; i++)
        platdev->device_id[i] = NULL;

    return platdev;
}

void
archi_opencl_platdev_set(
        archi_opencl_platdev_t *platdev,

        cl_uint platform_index,
        const cl_uint device_index[],

        ARCHI_ERROR_PARAM_DECL)
{
    if (platdev == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to platform/device IDs is NULL");
        return;
    }
    else if ((platdev->num_devices != 0) && (device_index == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of device indices is NULL");
        return;
    }

    // Obtain platform ID
    cl_platform_id platform_id;
    {
        cl_uint num_platforms;

        cl_int ret = clGetPlatformIDs(0, (cl_platform_id*)NULL, &num_platforms);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't get number of platform IDs");
            return;
        }

        if (platform_index >= num_platforms)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "platform index (%u) is out of valid range [0; %i)",
                    platform_index, num_platforms);
            return;
        }

        cl_platform_id *platform_list = malloc(sizeof(cl_platform_id) * num_platforms);
        if (platform_list == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of platform IDs (length = %u)",
                    num_platforms);
            return;
        }

        ret = clGetPlatformIDs(num_platforms, platform_list, (cl_uint*)NULL);
        if (ret != CL_SUCCESS)
        {
            free(platform_list);

            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't get platform IDs");
            return;
        }

        platform_id = platform_list[platform_index];

        free(platform_list);
    }

    // Obtain device IDs
    if (platdev->num_devices > 0)
    {
        cl_uint num_devices;

        cl_int ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL,
                0, (cl_device_id*)NULL, &num_devices);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't get number of device IDs");
            return;
        }

        for (cl_uint i = 0; i < num_devices; i++)
            if (device_index[i] >= num_devices)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "device index (%u) is out of valid range [0; %i)",
                        device_index[i], num_devices);
                return;
            }

        cl_device_id *device_list = malloc(sizeof(cl_device_id) * num_devices);
        if (device_list == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of device IDs (length = %u)",
                    num_devices);
            return;
        }

        ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL,
                num_devices, device_list, (cl_uint*)NULL);
        if (ret != CL_SUCCESS)
        {
            free(device_list);

            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't get device IDs");
            return;
        }

        // Write the output
        platdev->platform_id = platform_id;

        for (cl_uint i = 0; i < platdev->num_devices; i++)
            platdev->device_id[i] = device_list[device_index[i]];

        free(device_list);
    }

    ARCHI_ERROR_RESET();
}

