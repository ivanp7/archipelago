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
 * @brief Description of an executable input file.
 */

#pragma once
#ifndef _ARCHI_EXE_INPUT_TYP_H_
#define _ARCHI_EXE_INPUT_TYP_H_

#include "archi/res/file/header.typ.h"

struct archi_parameter_list;
struct archi_exe_registry_instr_list;

#define ARCHI_EXE_INPUT_MAGIC   "[archi]" ///< Magic bytes identifying the format of input files.

/**
 * @brief Description of an input file for the executable.
 *
 * @note Valid input files always have `magic` containing the same bytes as ARCHI_EXE_INPUT_MAGIC.
 */
typedef struct archi_exe_input_file_header {
    archi_file_header_t header; ///< Memory-mapped file header.
    char magic[8]; ///< Magic bytes identifying the file format.

    struct archi_parameter_list *params; ///< File parameter list.
    struct archi_exe_registry_instr_list *instructions; ///< List of instructions.
} archi_exe_input_file_header_t;

#endif // _ARCHI_EXE_INPUT_TYP_H_

