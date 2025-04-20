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
 * @brief Command line arguments parsing.
 */

#pragma once
#ifndef _ARCHI_EXE_ARGS_FUN_H_
#define _ARCHI_EXE_ARGS_FUN_H_

#include "archi/util/status.typ.h"

struct archi_exe_args;

/**
 * @brief Parse command line arguments.
 *
 * @return Zero on success, non-zero error code otherwise.
 */
archi_status_t
archi_exe_args_parse(
        struct archi_exe_args *args, ///< [out] Parsed values of command line arguments.

        int argc,    ///< [in] Number of command line arguments.
        char *argv[] ///< [in] Command line arguments.
);

#endif // _ARCHI_EXE_ARGS_FUN_H_

