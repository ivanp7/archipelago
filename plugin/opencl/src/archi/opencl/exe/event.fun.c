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
 * @brief DEG operation functions for OpenCL event operations.
 */

#include "archi/opencl/exe/event.fun.h"
#include "archi/opencl/api/event.typ.h"


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__opencl_event_wait)
{
    const archi_opencl_event_array_t *wait_list = data;

    if (wait_list == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL event wait operation parameters is NULL");
        return;
    }
    else if ((wait_list->num_events != 0) && (wait_list->event == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "event wait list is NULL");
        return;
    }
    else if (wait_list->num_events > (cl_uint)-1)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "event wait list length is doesn't fit into cl_uint");
        return;
    }

    // Wait for events
    cl_int ret = clWaitForEvents(wait_list->num_events, wait_list->event);

    if (ret != CL_SUCCESS)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't wait for OpenCL events: error %i", ret);
        return;
    }

    // Release the events
    for (size_t i = 0; i < wait_list->num_events; i++)
    {
        clReleaseEvent(wait_list->event[i]);
        wait_list->event[i] = NULL;
    }

    ARCHI_ERROR_RESET();
}

