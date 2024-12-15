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
 * @brief Shared library operations.
 */

#pragma once
#ifndef _ARCHI_UTIL_OS_LIB_FUN_H_
#define _ARCHI_UTIL_OS_LIB_FUN_H_

#include <stdbool.h>

/**
 * @brief Load shared library.
 *
 * @return Handle of the loaded library, or NULL in case of failure.
 */
void*
archi_library_load(
        const char *pathname, ///< [in] Path to the loaded library.

        bool lazy,  ///< [in] Whether to perform lazy binding.
        bool global ///< [in] Whether defined symbols are available in subsequently loaded libraries.
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

#endif // _ARCHI_UTIL_OS_LIB_FUN_H_

