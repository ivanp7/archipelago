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
 * @brief Data for operation functions for printing formatted messages to streams.
 */

#pragma once
#ifndef _ARCHI_FILE_EXE_PRINT_TYP_H_
#define _ARCHI_FILE_EXE_PRINT_TYP_H_

#include "archi_base/pointer.typ.h"

#include <stdio.h> // for FILE


/**
 * @brief Format string token: a string or a conversion specification + argument.
 *
 * Tokens are treated in the following way:
 * 1. if `conv_spec` is NULL, the token is ignored;
 * 2. if `conv_spec` doesn't begin with '%', the token is printed as `printf("%s", conv_spec)`;
 * 3. otherwise, `conv_spec` must be a valid printf conversion specification
 *    with no trailing characters. The token is printed as `printf(conv_spec, argument)`.
 *    The argument is interpreted according to the conversion specification.
 *    Dynamic field width and precision (set to `*`) are not supported.
 */
typedef struct archi_dexgraph_op_data__file_printf__token {
    const char *conv_spec; ///< Conversion specification.
    archi_pointer_t argument; ///< Conversion specification argument.
} archi_dexgraph_op_data__file_printf__token_t;

/**
 * @brief Operation function data: print a formatted string to the specified stream.
 *
 * Setting `stream` to NULL is equivalent to `stdout`.
 */
typedef struct archi_dexgraph_op_data__file_printf {
    FILE *stream; ///< Output stream.

    const size_t num_tokens; ///< Number of format string tokens.
    archi_dexgraph_op_data__file_printf__token_t token[]; ///< Array of format string tokens.
} archi_dexgraph_op_data__file_printf_t;

#endif // _ARCHI_FILE_EXE_PRINT_TYP_H_

