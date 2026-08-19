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
 * @brief Types for file and memory operations.
 */

#pragma once
#ifndef _ARCHI_FILE_API_HEADER_TYP_H_
#define _ARCHI_FILE_API_HEADER_TYP_H_

#include <stddef.h> // for size_t


/**
 * @brief Header of a memory-mapped file with support of pointers.
 *
 * `header.addr` must be equal to `&header`.
 * The full size of mapped memory is `sizeof(header) + header.size`.
 */
typedef struct archi_file_ptr_support_header {
    void *addr;  ///< Address of the mapped memory starting location.
    size_t size; ///< Size of the memory in bytes, excluding the header.
} archi_file_ptr_support_header_t;

#endif // _ARCHI_FILE_API_HEADER_TYP_H_

