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
#ifndef _ARCHI_PLUGIN_FILES_CONFIG_TYP_H_
#define _ARCHI_PLUGIN_FILES_CONFIG_TYP_H_

/**
 * @brief File configuration.
 */
typedef struct archi_file_config {
    const char *pathname; ///< Path to a file.
    const char *mode;     ///< File mode.
} archi_file_config_t;

/**
 * @brief File configuration key for the whole configuration structure.
 */
#define ARCHI_FILE_CONFIG_KEY "config"

/**
 * @brief File context configuration key -- pathname.
 */
#define ARCHI_FILE_CONFIG_KEY_PATHNAME "pathname"

/**
 * @brief File context configuration key -- file mode.
 */
#define ARCHI_FILE_CONFIG_KEY_MODE "mode"

#endif // _ARCHI_PLUGIN_FILES_CONFIG_TYP_H_

