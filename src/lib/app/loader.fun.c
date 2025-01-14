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
#include "archi/app/loader.typ.h"
#include "archi/util/os/lib.fun.h"
#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h>

ARCHI_CONTAINER_ELEMENT_FUNC(archi_app_load_library)
{
    if ((key == NULL) || (element == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    const archi_app_loader_library_t *library = element;
    archi_container_t *loaded_libraries = data;

    if ((library->key == NULL) || (library->pathname == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    void *handle = archi_library_load(library->pathname, library->lazy, library->global);
    if (handle == NULL)
        return ARCHI_ERROR_LOAD;

    code = archi_container_insert(*loaded_libraries, library->key, handle);
    if (code != 0)
    {
        archi_library_unload(handle);
        return code;
    }

    return 0;
}

ARCHI_CONTAINER_ELEMENT_FUNC(archi_app_unload_library)
{
    (void) key;
    (void) data;

    if (element == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_library_unload(element);

    return 0;
}

ARCHI_CONTAINER_ELEMENT_FUNC(archi_app_get_library_content)
{
    if ((key == NULL) || (element == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    const archi_app_loader_library_symbol_t *symbol = element;
    archi_app_get_library_content_data_t *func_data = data;

    if ((symbol->key == NULL) || (symbol->library_key == NULL) || (symbol->symbol_name == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    void *handle = NULL;
    code = archi_container_extract(func_data->loaded_libraries,
            symbol->library_key, &handle);
    if (code != 0)
        return code;

    void *content = archi_library_get_symbol(handle, symbol->symbol_name);
    if (content == NULL)
        return ARCHI_ERROR_SYMBOL;

    code = archi_container_insert(func_data->contents, symbol->key, content);
    if (code != 0)
        return code;

    return 0;
}

