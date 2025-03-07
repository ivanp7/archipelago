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

struct archi_plugin_file_context;

/**
 * @brief Get file descriptor from a context.
 *
 * @return File descriptor.
 */
int
archi_plugin_file_context_descriptor(
        struct archi_plugin_file_context *context ///< [in] File context.
);

/**
 * @brief Get mapped memory pointer and size from a context.
 *
 * @return Mapped memory pointer.
 */
void*
archi_plugin_file_context_mapped_memory(
        struct archi_plugin_file_context *context, ///< [in] File context.

        size_t *memory_size ///< [out] Mapped memory size.
);

#define ARCHI_PLUGIN_FILE_SLOT_FILE_DESCRIPTOR "fd" ///< Output slot: file descriptor.
#define ARCHI_PLUGIN_FILE_SLOT_MAPPED_MEMORY "mm" ///< Output slot: mapped memory.

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_file_context_init);   ///< File context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_file_context_final); ///< File context finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_plugin_file_context_get);     ///< File context slot getter function.
ARCHI_CONTEXT_ACT_FUNC(archi_plugin_file_context_act);     ///< File context action function.

extern
const archi_context_interface_t archi_plugin_file_context_interface; ///< File context interface functions.

/**
 * @brief Key of file context interface.
 */
#define ARCHI_PLUGIN_FILE_INTERFACE "file"

/**
 * @brief Action: map the file into memory.
 */
#define ARCHI_PLUGIN_FILE_ACTION_MAP "map"

/**
 * @brief Action parameter: whether to close the file descriptor.
 */
#define ARCHI_PLUGIN_FILE_ACTION_MAP_PARAM_CLOSE "close"

#endif // _ARCHI_PLUGIN_FILES_H_

