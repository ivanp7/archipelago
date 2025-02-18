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
 * @brief Type of parsed command line arguments.
 */

#pragma once
#ifndef _ARCHI_EXE_ARGS_TYP_H_
#define _ARCHI_EXE_ARGS_TYP_H_

#include <stdbool.h>

/**
 * @brief Values of command line arguments.
 */
typedef struct archi_args {
    const char *pathname; ///< Pathname of shared memory with configuration.
    int proj_id; ///< Project identifier of shared memory with configuration.

    int verbosity_level; ///< Application verbosity level.
    bool no_logo; ///< Whether to suppress display of the application logo.
} archi_args_t;

#endif // _ARCHI_EXE_ARGS_TYP_H_

