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
 * @brief Context configuration types of the plugin.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_SHARED_MEMORY_CONFIG_TYP_H_
#define _ARCHI_PLUGIN_SHARED_MEMORY_CONFIG_TYP_H_

#include <stdbool.h>

/**
 * @brief Shared memory configuration.
 */
typedef struct archi_shared_memory_config {
    const char *file; ///< Pathname of a memory-mapped file.

    bool readable; ///< Whether is shared memory readable.
    bool writable; ///< Whether is shared memory writable.
    bool shared;   ///< Whether updates to the mapping are visible to other processes.
    int flags;     ///< Other mmap() flags.
} archi_shared_memory_config_t;

/**
 * @brief Shared memory configuration key for the whole configuration structure.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY "config"

/**
 * @brief Shared memory configuration key -- pathname of a memory-mapped file.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY_FILE "file"

/**
 * @brief Shared memory configuration key -- whether is shared memory readable.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY_READABLE "readable"

/**
 * @brief Shared memory configuration key -- whether is shared memory writable.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY_WRITABLE "writable"

/**
 * @brief Shared memory configuration key -- whether updates to the mapping are visible to other processes.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY_SHARED "shared"

/**
 * @brief Shared memory configuration key -- other mmap() flags.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY_FLAGS "flags"

#endif // _ARCHI_PLUGIN_SHARED_MEMORY_CONFIG_TYP_H_

