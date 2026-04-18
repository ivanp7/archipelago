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
 * @brief Context interface for OpenCL devices.
 */

#include "archi/opencl/ctx/context.var.h"
#include "archi/opencl/api/device.fun.h"
#include "archi/opencl/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <CL/cl.h>

#include <stdlib.h> // for malloc(), free()


struct archi_context_data__opencl_context {
    archi_rcpointer_t context;
    archi_opencl_platdev_t *platdev;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__opencl_context)
{
    // Parse parameters
    cl_uint platform_idx = 0;
    const cl_uint *device_idx = NULL;
    size_t num_devices = 0;
    {
        archi_rcpointer_t devices = {0};

        archi_plist_param_t parsed[] = {
            {.name = "platform",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, cl_uint)}},
                .assign = {archi_plist_assign__value, &platform_idx, sizeof(platform_idx), NULL}},
            {.name = "device",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, cl_uint)}},
                .assign = {archi_plist_assign__rcpointer, &devices, sizeof(devices), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;

        device_idx = devices.cptr;
        archi_pointer_attr_unpk__pdata(devices.attr, &num_devices, NULL, NULL, NULL);
    }

    if (num_devices > (cl_uint)-1)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of devices doesn't fit into cl_uint");
        return NULL;
    }

    // Construct the context
    struct archi_context_data__opencl_context *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__opencl_context){
        .context = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT),
        },
        .platdev = archi_opencl_platdev_alloc(num_devices),
    };

    if (context_data->platdev == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate platform/device IDs storage object");
        return NULL;
    }

    archi_opencl_platdev_set(context_data->platdev,
            platform_idx, device_idx, ARCHI_ERROR_PARAM);

    if (context_data->platdev->platform_id == NULL)
    {
        free(context_data->platdev);
        free(context_data);
        return NULL;
    }

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)context_data->platdev->platform_id,
        0};

    cl_int ret;
    context_data->context.ptr = clCreateContext(properties,
            num_devices, context_data->platdev->device_id, NULL, NULL, &ret);

    if (context_data->context.ptr == NULL)
    {
        free(context_data->platdev);
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create an OpenCL context on platform #%u: error %i",
                platform_idx, ret);
        return NULL;
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__opencl_context)
{
    struct archi_context_data__opencl_context *context_data =
        (struct archi_context_data__opencl_context*)context;

    clReleaseContext(context_data->context.ptr);
    free(context_data->platdev);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__opencl_context)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__opencl_context *context_data =
        (struct archi_context_data__opencl_context*)context;

    if (ARCHI_STRING_COMPARE("platform_id", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_rcpointer_t value = {
            .ptr = context_data->platdev->platform_id,
            .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("device_id", ==, slot.name))
    {
        if (slot.num_indices > 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0 or 1");
            return;
        }

        archi_rcpointer_t value;

        if (slot.num_indices == 0)
        {
            value = (archi_rcpointer_t){
                .ptr = context_data->platdev->device_id,
                .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                    ARCHI_POINTER_ATTR__PDATA(context_data->platdev->num_devices, cl_device_id),
                .ref_count = ARCHI_CONTEXT_REF_COUNT,
            };
        }
        else
        {
            if ((slot.index[0] < 0) || ((size_t)slot.index[0] >= context_data->platdev->num_devices))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %u)",
                        slot.index[0], context_data->platdev->num_devices);
                return;
            }

            value = (archi_rcpointer_t){
                .ptr = context_data->platdev->device_id[slot.index[0]],
                .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID),
            };
        }

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__opencl_context = {
    .init_fn = archi_context_init__opencl_context,
    .final_fn = archi_context_final__opencl_context,
    .eval_fn = archi_context_eval__opencl_context,
};

