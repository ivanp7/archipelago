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
 * @brief Types for file and memory operations.
 */

#pragma once
#ifndef _ARCHI_RES_LIBRARY_API_TYP_H_
#define _ARCHI_RES_LIBRARY_API_TYP_H_

#include <stdbool.h>

/**
 * @brief Parameters for archi_library_load().
 */
typedef struct archi_library_load_params {
    const char *pathname; ///< Pathname of library file, or NULL for the main program.

    bool lazy;   ///< Whether to perform lazy binding.
    bool global; ///< Whether defined symbols are available in subsequently loaded libraries.

    int flags;   ///< Other flags for dlopen().
} archi_library_load_params_t;

#endif // _ARCHI_RES_LIBRARY_API_TYP_H_

