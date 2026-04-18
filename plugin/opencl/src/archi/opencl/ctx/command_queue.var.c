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
 * @brief Context interface for OpenCL command queues.
 */

#include "archi/opencl/ctx/command_queue.var.h"
#include "archi/opencl/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <CL/cl.h>

#include <stdlib.h> // for malloc(), free()


struct archi_context_data__opencl_command_queue {
    archi_rcpointer_t command_queue;
    cl_platform_id platform_id;
    cl_device_id device_id;

    // References
    archi_rcpointer_t ref_context;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__opencl_command_queue)
{
    // Parse parameters
    archi_rcpointer_t opencl_context = {0};
    cl_device_id device_id = NULL;
    bool out_of_order_exec = false, profiling = false;
#if defined(cl_khr_priority_hints)
    cl_queue_priority_khr priority_hint = 0;
#endif
#if defined(cl_khr_throttle_hints)
    cl_queue_throttle_khr throttle_hint = 0;
#endif
    {
        archi_plist_param_t parsed[] = {
            {.name = "context",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT)}},
                .assign = {archi_plist_assign__rcpointer, &opencl_context, sizeof(opencl_context), NULL}},
            {.name = "device_id",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID)}},
                .assign = {archi_plist_assign__dptr, &device_id, sizeof(device_id), NULL}},
            {.name = "out_of_order_exec",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &out_of_order_exec, sizeof(out_of_order_exec), NULL}},
            {.name = "profiling",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &profiling, sizeof(profiling), NULL}},
#if defined(cl_khr_priority_hints)
            {.name = "priority_hint",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, cl_queue_priority_khr)}},
                .assign = {archi_plist_assign__value, &priority_hint, sizeof(priority_hint), NULL}},
#endif
#if defined(cl_khr_throttle_hints)
            {.name = "throttle_hint",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, cl_queue_throttle_khr)}},
                .assign = {archi_plist_assign__value, &throttle_hint, sizeof(throttle_hint), NULL}},
#endif
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    if (opencl_context.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL context is not specified");
        return NULL;
    }
    else if (device_id == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL device ID is not specified");
        return NULL;
    }

    // Construct the context
    struct archi_context_data__opencl_command_queue *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__opencl_command_queue){
        .command_queue = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_COMMAND_QUEUE),
        },
        .device_id = device_id,
        .ref_context = archi_rcpointer_own(opencl_context, ARCHI_ERROR_PARAM),
    };

    if (!context_data->ref_context.attr)
    {
        free(context_data);
        return NULL;
    }

    // Get platform ID
    {
        cl_int ret = clGetDeviceInfo(device_id, CL_DEVICE_PLATFORM,
                sizeof(context_data->platform_id), &context_data->platform_id, NULL);
        if (ret != CL_SUCCESS)
        {
            archi_rcpointer_disown(context_data->ref_context);
            free(context_data);

            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't obtain platform ID of the devices");
            return NULL;
        }
    }

    // Create the command queue
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
    context_data->command_queue.ptr = clCreateCommandQueueWithProperties(
            opencl_context.ptr, device_id, properties, &ret);

    if (context_data->command_queue.ptr == NULL)
    {
        archi_rcpointer_disown(context_data->ref_context);
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create an OpenCL command queue: error %i", ret);
        return NULL;
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__opencl_command_queue)
{
    struct archi_context_data__opencl_command_queue *context_data =
        (struct archi_context_data__opencl_command_queue*)context;

    clReleaseCommandQueue(context_data->command_queue.ptr);
    archi_rcpointer_disown(context_data->ref_context);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__opencl_command_queue)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__opencl_command_queue *context_data =
        (struct archi_context_data__opencl_command_queue*)context;

    if (ARCHI_STRING_COMPARE("context", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->ref_context);
    }
    else if (ARCHI_STRING_COMPARE("platform_id", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_rcpointer_t value = {
            .ptr = context_data->platform_id,
            .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("device_id", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_rcpointer_t value = {
            .ptr = context_data->device_id,
            .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__opencl_command_queue = {
    .init_fn = archi_context_init__opencl_command_queue,
    .final_fn = archi_context_final__opencl_command_queue,
    .eval_fn = archi_context_eval__opencl_command_queue,
};

