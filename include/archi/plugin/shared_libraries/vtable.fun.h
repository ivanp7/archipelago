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
 * @brief Functions of the plugin for operations with shared libraries.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_SHARED_LIBRARIES_VTABLE_FUN_H_
#define _ARCHI_PLUGIN_SHARED_LIBRARIES_VTABLE_FUN_H_

#include "archi/app/plugin.def.h"

ARCHI_PLUGIN_HELP_FUNC(archi_shared_libraries_vtable_help_func);     ///< Help function.
ARCHI_PLUGIN_INIT_FUNC(archi_shared_libraries_vtable_init_func);     ///< Initialization function.
ARCHI_PLUGIN_FINAL_FUNC(archi_shared_libraries_vtable_final_func);   ///< Finalization function.
ARCHI_PLUGIN_GET_FUNC(archi_shared_libraries_vtable_get_func);       ///< Getter function.

#endif // _ARCHI_PLUGIN_SHARED_LIBRARIES_VTABLE_FUN_H_

