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
#include "archi/opencl/exe/event.typ.h"
#include "archi/opencl/api/event.fun.h"
#include "archi/timer/api/timer.fun.h"


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__opencl_event_wait)
{
    if (data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL event wait operation parameters is NULL");
        return;
    }

    archi_opencl_event_array_t wait_list = *(archi_opencl_event_array_t*)data;

    if (!archi_opencl_event_wait(wait_list, ARCHI_ERROR_PARAM))
        return;

    archi_opencl_event_release(wait_list);
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__opencl_event_profile)
{
    if (data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL event time recording operation parameters is NULL");
        return;
    }

    archi_dexgraph_op_data__opencl_event_profile_t *profiling_data = data;

    if (profiling_data->event == NULL)
    {
        ARCHI_ERROR_RESET();
        return;
    }

    if (profiling_data->timer == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "timer is NULL");
        return;
    }
    else if (profiling_data->from_time == profiling_data->to_time)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "specified time interval has zero size");
        return;
    }

    cl_ulong interval_start, interval_end; // nanoseconds
    {
        cl_int ret;

        ret = clGetEventProfilingInfo(profiling_data->event, profiling_data->from_time,
                sizeof(interval_start), &interval_start, NULL);

        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't get interval start time: error %i", ret);
            return;
        }

        ret = clGetEventProfilingInfo(profiling_data->event, profiling_data->to_time,
                sizeof(interval_end), &interval_end, NULL);

        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't get interval end time: error %i", ret);
            return;
        }
    }

    if (interval_start > interval_end)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "interval start time exceeds end time");
        return;
    }

    float seconds = (interval_end - interval_start) * 1e-9f;
    if (!archi_timer_record(profiling_data->timer, seconds))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "couldn't record time interval");
        return;
    }

    clReleaseEvent(profiling_data->event);
    profiling_data->event = NULL;

    ARCHI_ERROR_RESET();
}

