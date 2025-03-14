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
#include <stddef.h>

/**
 * @brief Values of command line arguments.
 */
typedef struct archi_args {
    char **inputs;     ///< Array of pathnames of input memory-mapped configuration files.
    size_t num_inputs; ///< Number of input configuration files.

    bool no_fsm; ///< Whether to skip FSM execution.

    bool no_logo; ///< Whether to suppress display of the application logo.
    int verbosity_level; ///< Application verbosity level.
} archi_args_t;

#endif // _ARCHI_EXE_ARGS_TYP_H_

