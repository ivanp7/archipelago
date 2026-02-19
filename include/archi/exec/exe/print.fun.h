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
 * @brief Operation functions for printing messages.
 */

#pragma once
#ifndef _ARCHI_EXEC_EXE_PRINT_FUN_H_
#define _ARCHI_EXEC_EXE_PRINT_FUN_H_

#include "archi/exec/api/operation.typ.h"


/**
 * @brief Operation function: print a string to the standard output stream.
 *
 * Function data type: archi_dexgraph_op_data__print_string_t.
 *
 * Tokens are treated in the following way:
 * 1. if `conv_spec` is NULL, token is ignored;
 * 2. if `conv_spec` doesn't begin with '%', it is printed as `printf("%s", conv_spec)`;
 * 3. otherwise, `conv_spec` must be a single valid printf conversion specification
 *    with no superfluous trailing characters.
 *    Value is interpreted according to the length modifier,
 *    and the node is printed as `printf(conv_spec, argument)`.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__print_string);

#endif // _ARCHI_EXEC_EXE_PRINT_FUN_H_

