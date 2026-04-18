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
    for (size_t i = 0; i < enqueue_data->wait_list.num_events; i++)
    {
        clReleaseEvent(enqueue_data->wait_list.event[i]);
        enqueue_data->wait_list.event[i] = NULL;
    }

    // Write output event to the designated locations
    if (enqueue_data->out_list.num_event_ptrs != 0)
    {
        for (size_t i = 0; i < enqueue_data->out_list.num_event_ptrs; i++)
        {
            if (enqueue_data->out_list.event_ptr[i] == NULL)
                continue;

            clRetainEvent(output_event);
            *enqueue_data->out_list.event_ptr[i] = output_event;
        }

        clReleaseEvent(output_event);
    }

    ARCHI_ERROR_RESET();
}

