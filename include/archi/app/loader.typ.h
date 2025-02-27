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
 * @brief Shared library loader configuration types.
 */

#pragma once
#ifndef _ARCHI_APP_LOADER_TYP_H_
#define _ARCHI_APP_LOADER_TYP_H_

#include "archi/util/os/library.typ.h"

/**
 * @brief Shared library loader configuration.
 */
typedef struct archi_app_loader_library {
    const char *key; ///< Library key.

    archi_library_load_config_t param; ///< Parameters for archi_library_load().
} archi_app_loader_library_t;

/**
 * @brief Shared library symbol getter configuration.
 */
typedef struct archi_app_loader_library_symbol {
    const char *key; ///< Symbol key.

    const char *symbol_name; ///< Symbol name.
    const char *library_key; ///< Library key.
} archi_app_loader_library_symbol_t;

#endif // _ARCHI_APP_LOADER_TYP_H_

