/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Types for application plugin loader.
 */

#pragma once
#ifndef _ARCHI_APP_LOADER_TYP_H_
#define _ARCHI_APP_LOADER_TYP_H_

#include <stdbool.h>

/**
 * @brief Loader configuration for a library.
 */
typedef struct archi_app_loader_library {
    const void *key; ///< Library handle key.

    const char *pathname; ///< Pathname of library file.
    bool lazy;   ///< Whether to perform lazy binding.
    bool global; ///< Whether defined symbols are available in subsequently loaded libraries.
} archi_app_loader_library_t;

/**
 * @brief Loader configuration for a library symbol.
 */
typedef struct archi_app_loader_library_symbol {
    const void *key; ///< Symbol address key.

    const void *library_key; ///< Library handle key.
    const char *symbol_name; ///< Symbol name.
} archi_app_loader_library_symbol_t;

#endif // _ARCHI_APP_LOADER_TYP_H_

