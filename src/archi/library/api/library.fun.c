/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
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

#include "archi/library/api/library.fun.h"

#include <dlfcn.h> // for dlopen(), dlclose(), dlsym()
#include <stddef.h> // for NULL


archi_library_handle_t
archi_library_load(
        const char *pathname,
        archi_library_load_params_t params,
        ARCHI_ERROR_PARAM_DECL)
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

    archi_library_handle_t library = dlopen(pathname, flags);
    if (library == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't load shared library");
        return NULL;
    }

    ARCHI_ERROR_RESET();
    return library;
}

void
archi_library_unload(
        archi_library_handle_t library)
{
    if (library == NULL)
        return;

    dlclose(library);
}

void*
archi_library_get_symbol(
        archi_library_handle_t library,
        const char *symbol)
{
    if ((library == NULL) || (symbol == NULL))
        return NULL;

    return dlsym(library, symbol);
}

