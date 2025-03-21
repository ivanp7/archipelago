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
 * @brief Shared library handles.
 */

#pragma once
#ifndef _ARCHI_PLUGIN_SHARED_LIBRARIES_H_
#define _ARCHI_PLUGIN_SHARED_LIBRARIES_H_

#include "archi/app/context.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_shared_library_context_init);   ///< Shared library context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_shared_library_context_final); ///< Shared library context finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_plugin_shared_library_context_get);     ///< Shared library context slot getter function.
ARCHI_CONTEXT_ACT_FUNC(archi_plugin_shared_library_context_act);     ///< Shared library context action function.

extern
const archi_context_interface_t archi_plugin_shared_library_context_interface; ///< Shared library context interface functions.

/**
 * @brief Key of shared library context interface.
 */
#define ARCHI_PLUGIN_SHARED_LIBRARY_INTERFACE "shared_library"

/**
 * @brief Action: set type of extracted symbols to 'function'.
 */
#define ARCHI_PLUGIN_SHARED_LIBRARY_ACTION_SYMTYPE_FUNC "sym_func"

/**
 * @brief Action: set type of extracted symbols to 'data'.
 */
#define ARCHI_PLUGIN_SHARED_LIBRARY_ACTION_SYMTYPE_DATA "sym_data"

#endif // _ARCHI_PLUGIN_SHARED_LIBRARIES_H_

