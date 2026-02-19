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
 * @brief Types for operations on OpenCL devices.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_DEVICE_TYP_H_
#define _ARCHI_OPENCL_API_DEVICE_TYP_H_

#include <CL/cl.h>


/**
 * @brief Platform and device IDs needed to create a context.
 */
typedef struct archi_opencl_platdev {
    cl_platform_id platform_id; ///< OpenCL platform ID.

    const cl_uint num_devices; ///< Length of OpenCL device ID array.
    cl_device_id device_id[];  ///< Array of OpenCL device IDs.
} archi_opencl_platdev_t;

#endif // _ARCHI_OPENCL_API_DEVICE_TYP_H_

