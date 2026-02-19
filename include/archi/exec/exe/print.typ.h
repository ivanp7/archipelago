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
 * @brief Data for operation functions for printing messages.
 */

#pragma once
#ifndef _ARCHI_EXEC_EXE_PRINT_TYP_H_
#define _ARCHI_EXEC_EXE_PRINT_TYP_H_

#include "archi_base/pointer.typ.h"


/**
 * @brief Conversion specifier + argument.
 */
typedef struct archi_dexgraph_op_data__print_string__token {
    const char *conv_spec; ///< printf() conversion specifier.
    archi_pointer_t argument; ///< printf() argument.
} archi_dexgraph_op_data__print_string__token_t;

/**
 * @brief Operation function data: print a string to the standard output stream.
 */
typedef struct archi_dexgraph_op_data__print_string {
    const size_t num_tokens; ///< Number of printed tokens.
    archi_dexgraph_op_data__print_string__token_t token[]; ///< Array of "conversion specifier + argument" pairs.
} archi_dexgraph_op_data__print_string_t;

#endif // _ARCHI_EXEC_EXE_PRINT_TYP_H_

