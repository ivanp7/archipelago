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

#pragma once
#ifndef _ARCHI_OPENCL_API_EVENT_FUN_H_
#define _ARCHI_OPENCL_API_EVENT_FUN_H_

#include "archi/opencl/api/event.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


/**
 * @brief Wait for a list of events.
 *
 * @return True on success, false on failure.
 */
bool
archi_opencl_event_wait(
        archi_opencl_event_array_t wait_list, ///< [in] Wait list.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Release events and reset pointers in the array.
 */
void
archi_opencl_event_release(
        archi_opencl_event_array_t wait_list ///< [in,out] Wait list.
);

/**
 * @brief Assign an event to the specified locations.
 *
 * Event is retained on each assignment.
 */
void
archi_opencl_event_assign(
        archi_opencl_event_ptr_array_t out_list, ///< [in] List of locations to assign the event to..
        cl_event event ///< [in] Event.
);

#endif // _ARCHI_OPENCL_API_EVENT_FUN_H_

