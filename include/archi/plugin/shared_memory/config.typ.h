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
    const char *pathname; ///< Pathname of shared memory key.
    int proj_id;   ///< Project identifier of shared memory key.

    bool writable; ///< Whether shared memory is writable.
} archi_shared_memory_config_t;

/**
 * @brief Shared memory configuration key for the whole configuration structure.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY "config"

/**
 * @brief Shared memory configuration key -- pathname.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY_PATHNAME "pathname"

/**
 * @brief Shared memory configuration key -- project identifier.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY_PROJECT_ID "proj_id"

/**
 * @brief Shared memory configuration key -- whether shared memory is writable.
 */
#define ARCHI_SHARED_MEMORY_CONFIG_KEY_WRITABLE "writable"

#endif // _ARCHI_PLUGIN_SHARED_MEMORY_CONFIG_TYP_H_

