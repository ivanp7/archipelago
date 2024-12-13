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
 * @brief Constants of the plugin for operations with shared memory.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_SHARED_MEMORY_VTABLE_DEF_H_
#define _ARCHI_PLUGIN_SHARED_MEMORY_VTABLE_DEF_H_

/**
 * @brief Plugin name.
 */
#define ARCHI_PLUGIN_SHARED_MEMORY_NAME "shared_memory"

/**
 * @brief Shared memory context configuration key -- pathname.
 */
#define ARCHI_SHARED_MEMORY_PATHNAME "pathname"
/**
 * @brief Shared memory context configuration key -- project identifier.
 */
#define ARCHI_SHARED_MEMORY_PROJECT_ID "proj_id"
/**
 * @brief Shared memory context configuration key -- whether shared memory is writable.
 */
#define ARCHI_SHARED_MEMORY_WRITABLE "writable"

#endif // _ARCHI_PLUGIN_SHARED_MEMORY_VTABLE_DEF_H_

