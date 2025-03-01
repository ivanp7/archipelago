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
 * @brief Built-in context interfaces.
 */

#include "archi/exe/builtin.var.h"

#include "archi/plugin/files.h"
#include "archi/plugin/shared_libraries.h"
#include "archi/plugin/threads.h"

const archi_context_interface_t *const archi_builtin_interfaces[] = {
    &archi_plugin_file_context_interface,
    &archi_plugin_shared_library_context_interface,
    &archi_plugin_thread_group_context_interface,
    &archi_plugin_queue_context_interface,
};

const char *const archi_builtin_interfaces_keys[] = {
    ARCHI_PLUGIN_FILE_INTERFACE,
    ARCHI_PLUGIN_SHARED_LIBRARY_INTERFACE,
    ARCHI_PLUGIN_THREAD_GROUP_INTERFACE,
    ARCHI_PLUGIN_QUEUE_INTERFACE,
};

const size_t archi_builtin_interfaces_num_of = sizeof(archi_builtin_interfaces) /
    sizeof(archi_builtin_interfaces[0]);

