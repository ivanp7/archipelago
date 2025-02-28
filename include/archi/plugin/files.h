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
 * @brief File descriptors and mapped memory.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_FILES_H_
#define _ARCHI_PLUGIN_FILES_H_

#include "archi/app/context.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_file_context_init);   ///< File context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_file_context_final); ///< File context finalization function.

extern
const archi_context_interface_t archi_plugin_file_context_interface; ///< File context interface functions.

/**
 * @brief Alias name of file context interface.
 */
#define ARCHI_PLUGIN_FILE_INTERFACE "file"

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_file_map_context_init);   ///< File mapping context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_file_map_context_final); ///< File mapping context finalization function.

extern
const archi_context_interface_t archi_plugin_file_map_context_interface; ///< File mapping context interface functions.

/**
 * @brief Alias name of file mapping context interface.
 */
#define ARCHI_PLUGIN_FILE_MAP_INTERFACE "file_map"

/**
 * @brief File mapping context configuration key -- pathname.
 */
#define ARCHI_PLUGIN_FILE_MAP_CONFIG_KEY_PATHNAME "pathname"

#endif // _ARCHI_PLUGIN_FILES_H_

