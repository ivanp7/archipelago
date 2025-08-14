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
 * @brief Application context interface for shared libraries.
 */

#pragma once
#ifndef _ARCHI_RES_LIBRARY_CTX_LIBRARY_VAR_H_
#define _ARCHI_RES_LIBRARY_CTX_LIBRARY_VAR_H_

#include "archipelago/context/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_library_init);   ///< Shared library context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_library_final); ///< Shared library context finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_res_library_get);     ///< Shared library context slot getter function.
ARCHI_CONTEXT_ACT_FUNC(archi_context_res_library_act);     ///< Shared library context action function.

extern
const archi_context_interface_t archi_context_res_library_interface; ///< Shared library context interface.

#endif // _ARCHI_RES_LIBRARY_CTX_LIBRARY_VAR_H_

