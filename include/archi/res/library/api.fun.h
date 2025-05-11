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
 * @brief Operations with shared libraries.
 */

#pragma once
#ifndef _ARCHI_RES_LIBRARY_API_FUN_H_
#define _ARCHI_RES_LIBRARY_API_FUN_H_

#include "archi/res/library/api.typ.h"

/**
 * @brief Load shared library.
 *
 * @return Handle of the loaded library, or NULL in case of failure.
 */
void*
archi_library_load(
        archi_library_load_params_t params ///< [in] Shared library loading parameters.
);

/**
 * @brief Unload shared library.
 */
void
archi_library_unload(
        void *handle ///< [in] Handle of the unloaded library.
);

/**
 * @brief Get a symbol from shared library.
 */
void*
archi_library_get_symbol(
        void *restrict handle, ///< [in] Handle of the library.
        const char *restrict symbol ///< [in] Symbol name.
);

#endif // _ARCHI_RES_LIBRARY_API_FUN_H_

