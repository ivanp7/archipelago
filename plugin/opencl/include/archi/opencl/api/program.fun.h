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
 * @brief Operations on OpenCL programs.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_PROGRAM_FUN_H_
#define _ARCHI_OPENCL_API_PROGRAM_FUN_H_

#include "archi/opencl/api/program.typ.h"
#include "archi/opencl/api/device.typ.h"
#include "archi_base/kvlist.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


/**
 * @brief Create a collection of program sources from a key-value list.
 *
 * List keys are must be pathnames of the corresponding files,
 * and should not repeat.
 *
 * @return OpenCL program sources.
 */
archi_opencl_program_src_t
archi_opencl_program_src_from_kvlist(
        const archi_kvlist_t *list, ///< [in] List of sources.
        bool copy_buffers, ///< [in] Whether to allocate copies of pathnames and contents.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Copy a collection of program sources.
 *
 * @return Copy of OpenCL program sources.
 */
archi_opencl_program_src_t
archi_opencl_program_src_copy(
        archi_opencl_program_src_t sources, ///< [in] Original sources.
        bool copy_buffers, ///< [in] Whether to allocate copies of pathnames and contents.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Deallocate sources an OpenCL program.
 */
void
archi_opencl_program_src_free(
        archi_opencl_program_src_t sources, ///< [in] Sources of an OpenCL program.
        bool free_buffers ///< [in] Whether to deallocate pathnames and contents.
);

/**
 * @brief Build an OpenCL program from sources.
 *
 * @return OpenCL program handle, or NULL in case of error.
 */
cl_program
archi_opencl_program_build(
        cl_context context,  ///< [in] Valid OpenCL context.
        cl_uint num_devices, ///< [in] Size of array of device IDs.
        const cl_device_id device_id[], ///< [in] Array of device IDs to build the program for.

        archi_opencl_program_src_t headers, ///< [in] Program headers.
        archi_opencl_program_src_t sources, ///< [in] Program sources.

        cl_uint num_libraries,        ///< [in] Number of libraries to link.
        const cl_program libraries[], ///< [in] Array of libraries to link.

        const char *cflags, ///< [in] Compiler flags.
        const char *lflags, ///< [in] Linker flags.

        bool logging, ///< [in] Enable progress logging.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/*****************************************************************************/

/**
 * @brief Extract binaries of an OpenCL program.
 *
 * Contents buffers and platform/device IDs storage
 * are allocated in this function and must be deallocated later.
 *
 * @return Binaries of an OpenCL program.
 */
archi_opencl_program_bin_t
archi_opencl_program_bin_extract(
        cl_program program, ///< [in] OpenCL program.
        archi_opencl_platdev_t **out_platdev, ///< [out] Platform/device IDs associated with program.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Copy a collection of program binaries.
 *
 * @return Copy of OpenCL program binaries.
 */
archi_opencl_program_bin_t
archi_opencl_program_bin_copy(
        archi_opencl_program_bin_t binaries, ///< [in] Original binaries.
        bool copy_buffers, ///< [in] Whether to allocate copies of contents.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Deallocate binaries of an OpenCL program.
 */
void
archi_opencl_program_bin_free(
        archi_opencl_program_bin_t binaries, ///< [in] Binaries of an OpenCL program.
        bool free_buffers ///< [in] Whether to deallocate contents.
);

/**
 * @brief Create an OpenCL program from binaries.
 *
 * @return OpenCL program handle, or NULL in case of error.
 */
cl_program
archi_opencl_program_create(
        cl_context context, ///< [in] OpenCL context.

        const archi_opencl_platdev_t *platdev, ///< [in] Platform/device IDs.
        archi_opencl_program_bin_t binaries, ///< [in] Binaries of an OpenCL program.
        bool build, ///< [in] Whether the building step should be performed.

        bool logging, ///< [in] Enable progress logging.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_OPENCL_API_PROGRAM_FUN_H_

