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
 * @brief Types of collections of OpenCL program sources/binaries.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_PROGRAM_TYP_H_
#define _ARCHI_OPENCL_API_PROGRAM_TYP_H_

#include <stddef.h> // for size_t


/**
 * @brief Collection of source files of an OpenCL program.
 */
typedef struct archi_opencl_program_src {
    size_t num_files; ///< Number of files.

    char **pathname; ///< Pathnames of files.
    char **content;  ///< Contents of files.
    size_t *size;    ///< Sizes of files.
} archi_opencl_program_src_t;

/**
 * @brief Collection of binaries of an OpenCL program.
 */
typedef struct archi_opencl_program_bin {
    size_t num_binaries; ///< Number of binaries.

    unsigned char **content; ///< Binaries for each device.
    size_t *size;            ///< Sizes of the binaries for each device.
} archi_opencl_program_bin_t;

#endif // _ARCHI_OPENCL_API_PROGRAM_TYP_H_

