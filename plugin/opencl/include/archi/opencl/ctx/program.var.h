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
 * @brief Context interface for OpenCL programs.
 */

#pragma once
#ifndef _ARCHI_OPENCL_CTX_PROGRAM_VAR_H_
#define _ARCHI_OPENCL_CTX_PROGRAM_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: OpenCL program built from sources.
 *
 * Initialization parameters:
 * - "context"      : (cl_context) OpenCL context
 * - "device_id"    : (cl_device_id[]) array of pointers to OpenCL device IDs
 * - "headers"      : (archi_kvlist_t) pathname-contents list of headers
 * - "sources"      : (archi_kvlist_t) pathname-contents list of sources
 * - "libraries"    : (cl_program[]) array of libraries to link
 * - "cflags"       : (char[]) program compilation flags
 * - "lflags"       : (char[]) program linking flags
 *
 * Getter slots:
 * - "context"              : (cl_context) OpenCL context
 * - "platform_id"          : (cl_platform_id) OpenCL platform ID
 * - "device_id"            : (cl_device_id[]) array of OpenCL device IDs
 * - "device_id" [index]    : (cl_device_id) OpenCL device ID #index
 * - "binary.size"          : (size_t[]) array of sizes of program binaries for devices
 * - "binary.size" [index]  : (size_t) program binary size for device #index
 * - "binary"               : (char*[]) array of program binaries for devices
 * - "binary" [index]       : (char[]) program binary for device #index
 */
extern
const archi_context_interface_t
archi_context_interface__opencl_program_src;

/**
 * @brief Context interface: OpenCL program created from binaries.
 *
 * Initialization parameters:
 * - "context"      : (cl_context) OpenCL context
 * - "device_id"    : (cl_device_id[]) array of pointers to OpenCL device IDs
 * - "binaries"     : (char*[]) array of program binaries for the devices
 * - "binary_sizes" : (size_t[]) array of sizeof of program binaries for the devices
 * - "build"        : (char) perform the building step
 *
 * Getter slots:
 * - "context"              : (cl_context) OpenCL context
 * - "platform_id"          : (cl_platform_id) OpenCL platform ID
 * - "device_id"            : (cl_device_id[]) array of OpenCL device IDs
 * - "device_id" [index]    : (cl_device_id) OpenCL device ID #index
 * - "binary.size"          : (size_t[]) array of sizes of program binaries for devices
 * - "binary.size" [index]  : (size_t) program binary size for device #index
 * - "binary"               : (char*[]) array of program binaries for devices
 * - "binary" [index]       : (char[]) program binary for device #index
 */
extern
const archi_context_interface_t
archi_context_interface__opencl_program_bin;

#endif // _ARCHI_OPENCL_CTX_PROGRAM_VAR_H_

