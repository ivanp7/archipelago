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
 * @brief DEG operation function for printing hashmaps.
 */

#pragma once
#ifndef _ARCHI_HASHMAP_EXE_PRINT_FUN_H_
#define _ARCHI_HASHMAP_EXE_PRINT_FUN_H_

#include "archi/exec/api/operation.typ.h"


/**
 * @brief Operation function: print hashmap key-value pairs.
 *
 * Function data type: archi_hashmap_t.
 *
 * The list of pairs is printed to the standard output stream.
 * Only value pointer address and attributes are printed.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__hashmap_print);

#endif // _ARCHI_HASHMAP_EXE_PRINT_FUN_H_

