/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
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
 * @brief DEG operation function for mapping/unmapping memory.
 */

#pragma once
#ifndef _ARCHI_MEMORY_EXE_MAP_FUN_H_
#define _ARCHI_MEMORY_EXE_MAP_FUN_H_

#include "archi/exec/api/operation.typ.h"


/**
 * @brief Operation function: wrapper for mapping and unmapping memory object.
 *
 * Function data type: archi_dexgraph_op_data__memory_map_unmap_wrapper_t.
 *
 * In case of mapping failure, the wrapped operation function is not called.
 * Mapped memory pointer is written to the specified data structure before the call,
 * and is reset to NULL afterwards.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__memory_map_unmap_wrapper);

#endif // _ARCHI_MEMORY_EXE_MAP_FUN_H_

