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
#ifndef _ARCHI_UTIL_OS_FILE_TYP_H_
#define _ARCHI_UTIL_OS_FILE_TYP_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Parameters for archi_file_open().
 */
typedef struct archi_file_open_config {
    const char *pathname; ///< Path to a file.

    bool readable; ///< Whether is file open to read.
    bool writable; ///< Whether is file open to write.
    bool nonblock; ///< Whether is file open in non-blocking mode.
    int flags;     ///< Other flags for open().
} archi_file_open_config_t;

/**
 * @brief File configuration key for the whole configuration structure.
 */
#define ARCHI_FILE_CONFIG_KEY "config"

/**
 * @brief File context configuration key -- pathname.
 */
#define ARCHI_FILE_CONFIG_KEY_PATHNAME "pathname"

/**
 * @brief File context configuration key -- whether is file open to read.
 */
#define ARCHI_FILE_CONFIG_KEY_READABLE "readable"

/**
 * @brief File context configuration key -- whether is file open to write.
 */
#define ARCHI_FILE_CONFIG_KEY_WRITABLE "writable"

/**
 * @brief File context configuration key -- whether is file open in non-blocking mode.
 */
#define ARCHI_FILE_CONFIG_KEY_NONBLOCK "nonblock"

/**
 * @brief File context configuration key -- other flags for open().
 */
#define ARCHI_FILE_CONFIG_KEY_FLAGS "flags"

/*****************************************************************************/

/**
 * @brief Parameters for archi_file_map().
 */
typedef struct archi_file_map_config {
    size_t size;   ///< Size of the mapped memory.
    size_t offset; ///< Offset in the file. Must be a multiple of the page size.

    bool readable; ///< Whether is shared memory readable.
    bool writable; ///< Whether is shared memory writable.
    bool shared;   ///< Whether updates to the mapping are visible to other processes.
    int flags;     ///< Other mmap() flags.
} archi_file_map_config_t;

/**
 * @brief File mapping parameter key for the whole structure of parameters.
 */
#define ARCHI_FILE_MAP_PARAM_KEY "params"

/**
 * @brief File mapping parameter key -- size of the mapped memory.
 */
#define ARCHI_FILE_MAP_PARAM_KEY_SIZE "size"

/**
 * @brief File mapping parameter key -- offset in the file.
 */
#define ARCHI_FILE_MAP_PARAM_KEY_OFFSET "offset"

/**
 * @brief File mapping parameter key -- whether is shared memory readable.
 */
#define ARCHI_FILE_MAP_PARAM_KEY_READABLE "readable"

/**
 * @brief File mapping parameter key -- whether is shared memory writable.
 */
#define ARCHI_FILE_MAP_PARAM_KEY_WRITABLE "writable"

/**
 * @brief File mapping parameter key -- whether updates to the mapping are visible to other processes.
 */
#define ARCHI_FILE_MAP_PARAM_KEY_SHARED "shared"

/**
 * @brief File mapping parameter key -- other mmap() flags.
 */
#define ARCHI_FILE_MAP_PARAM_KEY_FLAGS "flags"

/*****************************************************************************/

/**
 * @brief Mapped memory header.
 *
 * If header is an object in shared memory,
 * header.addr must be equal to &header.
 *
 * (header.end - header.addr) is the full size of shared memory.
 */
typedef struct archi_mmap_header {
    void *addr; ///< Address of the shared memory starting location.
    void *end;  ///< Address of the first location beyond shared memory end.
} archi_mmap_header_t;

#endif // _ARCHI_UTIL_OS_FILE_TYP_H_

