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
 * @brief Types for file operations.
 */

#pragma once
#ifndef _ARCHI_FILE_API_FILE_TYP_H_
#define _ARCHI_FILE_API_FILE_TYP_H_

#include <stdio.h> // for FILE
#include <stddef.h> // for size_t
#include <stdbool.h>

/**
 * @brief File descriptor.
 */
typedef int archi_file_descriptor_t;

/**
 * @brief File stream handle.
 */
typedef FILE *archi_file_stream_t;

/**
 * @brief Data type tag for archi_file_stream_t.
 */
#define ARCHI_POINTER_DATA_TAG__FILE_STREAM     7

/**
 * @brief Parameters for archi_file_open().
 */
typedef struct archi_file_open_params {
    size_t size; ///< Size of file.

    bool readable; ///< Whether is file open to read.
    bool writable; ///< Whether is file open to write.

    bool create;    ///< Whether file creation is allowed.
    bool exclusive; ///< Whether file creation is ensured.
    bool truncate;  ///< Whether file is truncated.
    bool append;    ///< Whether file is opened in append mode.

    int flags; ///< Other flags.
    int mode;  ///< File mode bits.
} archi_file_open_params_t;

/**
 * @brief Parameters for archi_file_map().
 *
 * If has_header is true, size is ignored (it is computed from the header).
 * Otherwise, size = 0 means "until the end of the file".
 */
typedef struct archi_file_map_params {
    size_t size;     ///< Size of the mapped memory.
    size_t offset;   ///< Offset in the file. Must be a multiple of the page size.

    bool ptr_support; ///< Whether the mapped memory has the pointer support header.

    bool readable; ///< Whether is mapped memory readable.
    bool writable; ///< Whether is mapped memory writable.
    bool shared;   ///< Whether updates to the mapping are visible to other processes.

    int flags;     ///< Other flags.
} archi_file_map_params_t;

#endif // _ARCHI_FILE_API_FILE_TYP_H_

