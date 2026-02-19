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
 * @brief Description of an application input file.
 */

#pragma once
#ifndef _ARCHI_APP_INPUT_FILE_TYP_H_
#define _ARCHI_APP_INPUT_FILE_TYP_H_

#include "archi/file/api/header.typ.h"
#include "archi_base/kvlist.typ.h"


#define ARCHI_APP_INPUT_FILE_MAGIC  "[archi]" ///< Magic bytes identifying the format of input files.

/**
 * @brief Header of an application input file.
 *
 * @note Valid input files always have `magic` containing the same bytes as ARCHI_APP_INPUT_FILE_MAGIC.
 */
typedef struct archi_app_input_file_header {
    archi_file_ptr_support_header_t header; ///< Header of a memory-mapped file.
    char magic[8]; ///< Magic bytes identifying the file format.

    archi_kvlist_t *contents; ///< File contents list.
} archi_app_input_file_header_t;

#endif // _ARCHI_APP_INPUT_FILE_TYP_H_

