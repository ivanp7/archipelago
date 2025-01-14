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
 * @brief Context interface of the plugin.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_SHARED_MEMORY_CONTEXT_FUN_H_
#define _ARCHI_PLUGIN_SHARED_MEMORY_CONTEXT_FUN_H_

#include "archi/app/context.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_shared_memory_context_init);   ///< Context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_shared_memory_context_final); ///< Context finalization function.

extern
const archi_context_interface_t archi_shared_memory_context_interface; ///< Context interface functions.

/**
 * @brief Plugin interface alias name.
 */
#define ARCHI_SHARED_MEMORY_CONTEXT_INTERFACE_ALIAS "shared_memory"

#endif // _ARCHI_PLUGIN_SHARED_MEMORY_CONTEXT_FUN_H_

