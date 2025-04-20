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
 * @brief Description of an application initialization file (executable input file).
 */

#pragma once
#ifndef _ARCHI_EXE_INPUT_TYP_H_
#define _ARCHI_EXE_INPUT_TYP_H_

#include "archi/res/file/header.typ.h"

struct archi_context_parameter_list;
struct archi_app_instruction_list;

/**
 * @brief Description of an input file for the executable.
 */
typedef struct archi_exe_input {
    archi_file_header_t header; ///< Memory-mapped file header.

    struct archi_context_parameter_list *params; ///< File parameter list.
    struct archi_app_instruction_list *instruction; ///< List of instructions.
} archi_exe_input_t;

#endif // _ARCHI_EXE_INPUT_TYP_H_

