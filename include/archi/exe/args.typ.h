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
 * @brief Command line arguments.
 */

#pragma once
#ifndef _ARCHI_EXE_ARGS_TYP_H_
#define _ARCHI_EXE_ARGS_TYP_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Parsed values of command line arguments.
 */
typedef struct archi_exe_args {
    // Functionality
    char **inputs;     ///< Array of pathnames of input memory-mapped initialization files.
    size_t num_inputs; ///< Number of input initialization files.

    bool dry_run; ///< Whether dry run is done: initialization instructions are logged only, not executed.
    bool no_signals; ///< Whether signal management is disabled.

    // Logging
    bool no_logo;  ///< Whether display of the application logo is suppressed.
    bool no_color; ///< Whether use of colors for log messages is disabled.
    int verbosity_level; ///< Application verbosity level.
} archi_exe_args_t;

#endif // _ARCHI_EXE_ARGS_TYP_H_

