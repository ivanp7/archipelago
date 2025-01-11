/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
#ifndef _ARCHI_PLUGIN_SHARED_LIBRARIES_CONFIG_TYP_H_
#define _ARCHI_PLUGIN_SHARED_LIBRARIES_CONFIG_TYP_H_

#include <stdbool.h>

/**
 * @brief Shared library configuration.
 */
typedef struct archi_shared_library_config {
    const char *pathname; ///< Pathname of library file.

    bool lazy;   ///< Whether to perform lazy binding.
    bool global; ///< Whether defined symbols are available in subsequently loaded libraries.
} archi_shared_library_config_t;

/**
 * @brief Shared library configuration key for the whole configuration structure.
 */
#define ARCHI_SHARED_LIBRARY_CONFIG_KEY "config"

/**
 * @brief Shared library context configuration key -- path to library file.
 */
#define ARCHI_SHARED_LIBRARY_CONFIG_KEY_PATHNAME "pathname"

/**
 * @brief Shared library context configuration key -- whether to perform lazy binding.
 */
#define ARCHI_SHARED_LIBRARY_CONFIG_KEY_LAZY "lazy"

/**
 * @brief Shared library context configuration key -- whether defined symbols are available in subsequently loaded libraries.
 */
#define ARCHI_SHARED_LIBRARY_CONFIG_KEY_GLOBAL "global"

#endif // _ARCHI_PLUGIN_SHARED_LIBRARIES_CONFIG_TYP_H_

