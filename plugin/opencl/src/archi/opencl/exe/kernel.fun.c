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
 * @brief DEG operation functions for OpenCL kernel operations.
 */

#include "archi/opencl/exe/kernel.fun.h"
#include "archi/opencl/exe/kernel.typ.h"
#include "archi/opencl/api/event.fun.h"
#include "archi/opencl/api/tag.def.h"
#include "archi_base/pointer.fun.h"


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__opencl_kernel_set_arg_value)
{
    const archi_dexgraph_op_data__opencl_kernel_set_argument_t *setarg_data = data;

    if (setarg_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL kernel argument setting operation parameters is NULL");
        return;
    }
    else if (setarg_data->kernel == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL kernel is NULL");
        return;
    }

    size_t length, stride;
    if (!archi_pointer_attr_unpk__pdata(setarg_data->value.attr, &length, &stride, NULL, ARCHI_ERROR_PARAM))
        return;

    const void *arg_value = setarg_data->value.cptr;
    if (length == 0)
        arg_value = NULL;

    cl_int ret = clSetKernelArg(setarg_data->kernel, setarg_data->arg_index, length * stride, arg_value);

    if (ret != CL_SUCCESS)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't set value of an OpenCL kernel argument #%u: error %i",
                setarg_data->arg_index, ret);
        return;
    }

    ARCHI_ERROR_RESET();
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__opencl_kernel_set_arg_svmptr)
{
    const archi_dexgraph_op_data__opencl_kernel_set_argument_t *setarg_data = data;

    if (setarg_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL kernel argument setting operation parameters is NULL");
        return;
    }
    else if (setarg_data->kernel == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL kernel is NULL");
        return;
    }
    else if (!archi_pointer_attr_compatible(setarg_data->value.attr,
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_SVM)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL kernel argument is not an SVM pointer");
        return;
    }

    cl_int ret = clSetKernelArgSVMPointer(setarg_data->kernel, setarg_data->arg_index, setarg_data->value.cptr);

    if (ret != CL_SUCCESS)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't set SVM pointer of an OpenCL kernel argument #%u: error %i",
                setarg_data->arg_index, ret);
        return;
    }

    ARCHI_ERROR_RESET();
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__opencl_kernel_enqueue)
{
    const archi_dexgraph_op_data__opencl_kernel_enqueue_t *enqueue_data = data;

    if (enqueue_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL kernel enqueue operation parameters is NULL");
        return;
    }
    else if (enqueue_data->command_queue == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL command queue is NULL");
        return;
    }
    else if (enqueue_data->kernel == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL kernel is NULL");
        return;
    }
    else if (enqueue_data->num_work_dimensions == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of work dimensions is zero");
        return;
    }
    else if (enqueue_data->num_work_dimensions > CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of work dimensions is greater than CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS");
        return;
    }
    else if ((enqueue_data->wait_list.num_events != 0) && (enqueue_data->wait_list.event == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "event wait list is NULL");
        return;
    }
    else if (enqueue_data->wait_list.num_events > (cl_uint)-1)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "event wait list length is doesn't fit into cl_uint");
        return;
    }
    else if ((enqueue_data->out_list.num_event_ptrs != 0) && (enqueue_data->out_list.event_ptr == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of pointers to locations to write output event to is NULL");
        return;
    }

    // Enqueue the kernel
    cl_event output_event;

    cl_int ret = clEnqueueNDRangeKernel(enqueue_data->command_queue, enqueue_data->kernel,
            enqueue_data->num_work_dimensions, enqueue_data->work_offset_global,
            enqueue_data->work_size_global, enqueue_data->work_size_local,
            enqueue_data->wait_list.num_events, enqueue_data->wait_list.event,
            enqueue_data->out_list.num_event_ptrs != 0 ? &output_event : NULL);

    if (ret != CL_SUCCESS)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't enqueue an OpenCL kernel: error %i", ret);
        return;
    }

    // Release the events
    archi_opencl_event_release(enqueue_data->wait_list);

    // Write output event to the designated locations
    if (enqueue_data->out_list.num_event_ptrs != 0)
    {
        archi_opencl_event_assign(enqueue_data->out_list, output_event);
        clReleaseEvent(output_event);
    }

    ARCHI_ERROR_RESET();
}

