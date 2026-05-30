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
 * @brief Data for DEG operation functions for OpenCL event operations.
 */

#pragma once
#ifndef _ARCHI_OPENCL_EXE_EVENT_TYP_H_
#define _ARCHI_OPENCL_EXE_EVENT_TYP_H_

#include "archi/timer/api/handle.typ.h"

#include <CL/cl.h>


/**
 * @brief Operation function data: record command execution time using a timer.
 *
 * `from_time` must be less than `to_time`.
 */
typedef struct archi_dexgraph_op_data__opencl_event_profile {
    cl_event event; ///< Event produced by an OpenCL command.
    archi_timer_t timer; ///< Timer.

    cl_profiling_info from_time; ///< Initial time of the recorded time inverval.
    cl_profiling_info to_time;   ///< Final time of the recorded time interval.
} archi_dexgraph_op_data__opencl_event_profile_t;

#endif // _ARCHI_OPENCL_EXE_EVENT_TYP_H_

