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
 * @brief Operations with OpenCL events.
 */

#include "archi/opencl/api/event.fun.h"


bool
archi_opencl_event_wait(
        archi_opencl_event_array_t wait_list,
        ARCHI_ERROR_PARAM_DECL)
{
    if (wait_list.num_events == 0)
    {
        ARCHI_ERROR_RESET();
        return true;
    }

    if (wait_list.event == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "event wait list is NULL");
        return false;
    }
    else if (wait_list.num_events > (cl_uint)-1)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "event wait list length is doesn't fit into cl_uint");
        return false;
    }

    // Wait for events
    cl_int ret = clWaitForEvents(wait_list.num_events, wait_list.event);

    if (ret != CL_SUCCESS)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't wait for OpenCL events: error %i", ret);
        return false;
    }

    ARCHI_ERROR_RESET();
    return true;
}

void
archi_opencl_event_release(
        archi_opencl_event_array_t wait_list)
{
    if (wait_list.event == NULL)
        return;

    for (size_t i = 0; i < wait_list.num_events; i++)
    {
        if (wait_list.event[i] == NULL)
            continue;

        clReleaseEvent(wait_list.event[i]);
        wait_list.event[i] = NULL;
    }
}

void
archi_opencl_event_assign(
        archi_opencl_event_ptr_array_t out_list,
        cl_event event)
{
    if (out_list.event_ptr == NULL)
        return;

    for (size_t i = 0; i < out_list.num_event_ptrs; i++)
    {
        if (out_list.event_ptr[i] == NULL)
            continue;

        clRetainEvent(event);
        *out_list.event_ptr[i] = event;
    }
}

