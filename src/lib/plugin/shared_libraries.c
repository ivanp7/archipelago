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
 * @brief Built-in plugin for loading shared libraries.
 */

#include "archi/plugin/shared_libraries/vtable.fun.h"
#include "archi/plugin/shared_libraries/vtable.def.h"
#include "archi/plugin/shared_libraries/vtable.var.h"
#include "archi/plugin/shared_libraries/interface.typ.h"
#include "archi/util/os/lib.fun.h"
#include "archi/app/plugin.def.h"
#include "archi/app/version.def.h"
#include "archi/util/list.fun.h"
#include "archi/util/list.def.h"
#include "archi/util/value.typ.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"

#include <string.h> // for strcmp()

const archi_plugin_vtable_t archi_vtable_shared_libraries = {
    .format = {.magic = ARCHI_API_MAGIC, .version = ARCHI_API_VERSION},
    .info = {.name = ARCHI_PLUGIN_SHARED_LIBRARIES_NAME,
        .description = "Operations with shared libraries.",
        .help_fn = archi_shared_libraries_vtable_help_func,
    },
    .func = {
        .init_fn = archi_shared_libraries_vtable_init_func,
        .final_fn = archi_shared_libraries_vtable_final_func,
        .get_fn = archi_shared_libraries_vtable_get_func,
    },
};

ARCHI_PLUGIN_HELP_FUNC(archi_shared_libraries_vtable_help_func)
{
    (void) topic;
    archi_print("\
This plugin provides contexts which are handles of loaded shared libraries.\n\
The getter function allows to get addresses of library symbols.\n\
\n\
Configuration options:\n\
    \"shared_library\": char[]\n\
or\n\
    \"pathname\": char[] -- path to the library file\n\
");
    return 0;
}

static
ARCHI_LIST_ACT_FUNC(archi_shared_libraries_vtable_init_func_config)
{
    archi_list_node_named_value_t *vnode = (archi_list_node_named_value_t*)node;
    archi_shared_library_config_t *config = data;

    if (strcmp(vnode->base.name, ARCHI_PLUGIN_SHARED_LIBRARIES_NAME) == 0) // whole configuration
    {
        if ((vnode->value.type != ARCHI_VALUE_DATA) || (vnode->value.ptr == NULL) ||
                (vnode->value.size != sizeof(*config)) || (vnode->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, vnode->value.ptr, vnode->value.size);
        return 0;
    }
    else if (strcmp(vnode->base.name, ARCHI_SHARED_LIBRARY_PATHNAME) == 0)
    {
        if ((vnode->value.type != ARCHI_VALUE_STRING) || (vnode->value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->pathname = vnode->value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_PLUGIN_INIT_FUNC(archi_shared_libraries_vtable_init_func)
{
    if ((context == NULL) || (config == NULL))
        return ARCHI_ERROR_MISUSE;

    const char *pathname = NULL;
    {
        archi_status_t code = archi_list_traverse((archi_list_t*)config, NULL, NULL,
                archi_shared_libraries_vtable_init_func_config, &pathname, true, 0, NULL); // start from head
        if (code != 0)
            return code;
    }

    if (pathname == NULL)
        return ARCHI_ERROR_CONFIG;

    void *handle = archi_library_load(pathname);
    if (handle == NULL)
        return ARCHI_ERROR_LOAD;

    *context = handle;
    return 0;
}

ARCHI_PLUGIN_FINAL_FUNC(archi_shared_libraries_vtable_final_func)
{
    archi_library_unload(context);
}

ARCHI_PLUGIN_GET_FUNC(archi_shared_libraries_vtable_get_func)
{
    if ((context == NULL) || (slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    void *symbol = archi_library_get_symbol(context, slot);
    if (symbol == NULL)
        return ARCHI_ERROR_SYMBOL;

    *value = (archi_value_t){
        .ptr = symbol, .size = 0, .num_of = 1, .type = ARCHI_VALUE_DATA,
    };

    return 0;
}

