/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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
 * @brief Functions for getting OpenCL status names from codes.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_STATUS_FUN_H_
#define _ARCHIP_OPENCL_STATUS_FUN_H_

#include <CL/cl.h>

/**
 * @brief Get OpenCL error name by value.
 *
 * @return OpenCL error name, or NULL for unknown code.
 */
const char*
archip_opencl_error_string(
        cl_int err ///< Error code returned by an OpenCL operation.
);

/**
 * @brief Get OpenCL program build status name by value.
 *
 * @return OpenCL build status name.
 */
const char*
archip_opencl_build_status_string(
        cl_build_status status ///< Program build status.
);

#endif // _ARCHIP_OPENCL_STATUS_FUN_H_

