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
 * @brief Operation functions for printing formatted messages to streams.
 */

#pragma once
#ifndef _ARCHI_FILE_EXE_PRINT_FUN_H_
#define _ARCHI_FILE_EXE_PRINT_FUN_H_

#include "archi/exec/i/operation.typ.h"


/**
 * @brief Operation function: print a string to the specified stream.
 *
 * Function data type: archi_dexgraph_op_data__file_printf_t.
 *
 * This function uses series of fprintf() calls to do the job.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__file_printf);

#endif // _ARCHI_FILE_EXE_PRINT_FUN_H_

