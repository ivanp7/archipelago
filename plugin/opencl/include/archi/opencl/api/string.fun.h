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
 * @brief Functions for converting OpenCL status codes to strings.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_STRING_FUN_H_
#define _ARCHI_OPENCL_API_STRING_FUN_H_

#include <CL/cl.h>


/**
 * @brief Get OpenCL error string by code.
 *
 * @return OpenCL error string, or NULL for unknown code.
 */
const char*
archi_opencl_string_error(
        cl_int err ///< Error code returned by an OpenCL operation.
);

/**
 * @brief Get OpenCL program build status string by code.
 *
 * @return OpenCL build status string.
 */
const char*
archi_opencl_string_build_status(
        cl_build_status status ///< Program build status.
);

#endif // _ARCHI_OPENCL_API_STRING_FUN_H_

