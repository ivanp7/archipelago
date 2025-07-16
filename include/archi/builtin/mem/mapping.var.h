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
 * @brief Application context interface for memory mapping.
 */

#pragma once
#ifndef _ARCHI_BUILTIN_MEM_MAPPING_VAR_H_
#define _ARCHI_BUILTIN_MEM_MAPPING_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_memory_mapping_init);   ///< Memory mapping context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_memory_mapping_final); ///< Memory mapping context finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_memory_mapping_get);     ///< Memory mapping context getter function.
ARCHI_CONTEXT_ACT_FUNC(archi_context_memory_mapping_act);     ///< Memory mapping context action function.

extern
const archi_context_interface_t archi_context_memory_mapping_interface; ///< Memory mapping context interface.

#endif // _ARCHI_BUILTIN_MEM_MAPPING_VAR_H_

