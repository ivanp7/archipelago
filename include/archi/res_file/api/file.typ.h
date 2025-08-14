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
 * @brief Types for file and memory operations.
 */

#pragma once
#ifndef _ARCHI_RES_FILE_API_FILE_TYP_H_
#define _ARCHI_RES_FILE_API_FILE_TYP_H_

#include <stdbool.h>
#include <stddef.h> // for size_t

/**
 * @brief Parameters for archi_file_open().
 */
typedef struct archi_file_open_params {
    const char *pathname; ///< Path to file.

    size_t size; ///< Size of file.

    bool create;    ///< Whether file creation is allowed.
    bool exclusive; ///< Whether file creation is ensured.
    bool truncate;  ///< Whether file is truncated.

    bool readable; ///< Whether is file open to read.
    bool writable; ///< Whether is file open to write.
    bool nonblock; ///< Whether is file open in non-blocking mode.

    int flags; ///< Other flags.
    int mode;  ///< Created file mode.
} archi_file_open_params_t;

#endif // _ARCHI_RES_FILE_API_FILE_TYP_H_

