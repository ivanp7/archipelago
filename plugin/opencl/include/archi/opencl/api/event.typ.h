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
 * @brief Types for OpenCL operation events.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_EVENT_TYP_H_
#define _ARCHI_OPENCL_API_EVENT_TYP_H_

#include <CL/cl.h>


/**
 * @brief Array of OpenCL operation events.
 */
typedef struct archi_opencl_event_array {
    size_t num_events; ///< Wait list length.
    cl_event *event;   ///< Array of events to wait for.
} archi_opencl_event_array_t;

/**
 * @brief Array of pointers to OpenCL operation events.
 */
typedef struct archi_opencl_event_ptr_array {
    size_t num_event_ptrs; ///< Number of pointers to output event.
    cl_event **event_ptr;  ///< Array of output event pointers.
} archi_opencl_event_ptr_array_t;

#endif // _ARCHI_OPENCL_API_EVENT_TYP_H_

