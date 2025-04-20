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

#include "archi/res/library/interface.fun.h"

#include <dlfcn.h> // for dlopen(), dlclose(), dlsym()
#include <stddef.h> // for NULL

void*
archi_library_load(
        archi_library_load_params_t params)
{
    int flags = params.flags;

    if (params.lazy)
        flags |= RTLD_LAZY;
    else
        flags |= RTLD_NOW;

    if (params.global)
        flags |= RTLD_GLOBAL;
    else
        flags |= RTLD_LOCAL;

    return dlopen(params.pathname, flags);
}

void
archi_library_unload(
        void *handle)
{
    if (handle != NULL)
        dlclose(handle);
}

void*
archi_library_get_symbol(
        void *restrict handle,
        const char *restrict symbol)
{
    if ((handle == NULL) || (symbol == NULL))
        return NULL;

    return dlsym(handle, symbol);
}

