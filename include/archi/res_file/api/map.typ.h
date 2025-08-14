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
 * @brief Types for file mapping operations.
 */

#pragma once
#ifndef _ARCHI_RES_FILE_API_MAP_TYP_H_
#define _ARCHI_RES_FILE_API_MAP_TYP_H_

#include <stdbool.h>
#include <stddef.h> // for size_t

/**
 * @brief Parameters for archi_file_map().
 *
 * If has_header is true, size is ignored (it is computed from the header).
 * Otherwise, size = 0 means "until the end of the file".
 */
typedef struct archi_file_map_params {
    size_t size;     ///< Size of the mapped memory.
    size_t offset;   ///< Offset in the file. Must be a multiple of the page size.

    bool has_header; ///< Whether the mapped memory has the header.

    bool readable; ///< Whether is mapped memory readable.
    bool writable; ///< Whether is mapped memory writable.
    bool shared;   ///< Whether updates to the mapping are visible to other processes.

    int flags;     ///< Other mmap() flags.
} archi_file_map_params_t;

#endif // _ARCHI_RES_FILE_API_MAP_TYP_H_

