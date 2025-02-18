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
 * @brief Application plugin loader operations.
 */

#include "archi/app/loader.fun.h"
#include "archi/app/instance.typ.h"
#include "archi/util/os/lib.fun.h"
#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h>

archi_status_t
archi_app_add_library(
        archi_application_t *app,
        archi_app_loader_library_t lib)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (lib.key == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    void *handle = archi_library_load(lib.pathname, lib.lazy, lib.global);
    if (handle == NULL)
        return ARCHI_ERROR_LOAD;

    code = archi_container_insert(app->libraries, lib.key, handle);
    if (code != 0)
    {
        archi_library_unload(handle);
        return code;
    }

    return 0;
}

archi_status_t
archi_app_remove_library(
        archi_application_t *app,
        const void *key)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (key == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    void *handle = NULL;

    code = archi_container_remove(app->libraries, key, &handle);
    if (code != 0)
        return code;

    if (handle == NULL)
        return ARCHI_ERROR_LOAD;

    archi_library_unload(handle);

    return 0;
}

archi_status_t
archi_app_add_interface(
        archi_application_t *app,
        archi_app_loader_library_symbol_t sym)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;
    else if ((sym.key == NULL) || (sym.library_key == NULL) || (sym.symbol_name == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    void *handle = NULL;
    code = archi_container_extract(app->libraries, sym.library_key, &handle);
    if (code != 0)
        return code;

    void *symbol = archi_library_get_symbol(handle, sym.symbol_name);
    if (symbol == NULL)
        return ARCHI_ERROR_SYMBOL;

    code = archi_container_insert(app->interfaces, sym.key, symbol);
    if (code != 0)
        return code;

    return 0;
}

archi_status_t
archi_app_remove_interface(
        archi_application_t *app,
        const void *key)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (key == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    code = archi_container_remove(app->interfaces, key, NULL);
    if (code != 0)
        return code;

    return 0;
}

