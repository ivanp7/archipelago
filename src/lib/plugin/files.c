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
 * @brief Built-in plugin for opening files.
 */

#include "archi/plugin/files/vtable.fun.h"
#include "archi/plugin/files/vtable.def.h"
#include "archi/plugin/files/vtable.var.h"
#include "archi/plugin/files/interface.typ.h"
#include "archi/util/os/shm.fun.h"
#include "archi/app/plugin.def.h"
#include "archi/app/version.def.h"
#include "archi/util/list.fun.h"
#include "archi/util/list.def.h"
#include "archi/util/value.typ.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"

#include <stdio.h> // for FILE, fopen(), fclose()
#include <string.h> // for strcmp()

const archi_plugin_vtable_t archi_vtable_files = {
    .format = {.magic = ARCHI_API_MAGIC, .version = ARCHI_API_VERSION},
    .info = {.name = ARCHI_PLUGIN_FILES_NAME,
        .description = "Operations with files.",
        .help_fn = archi_files_vtable_help_func,
    },
    .func = {
        .init_fn = archi_files_vtable_init_func,
        .final_fn = archi_files_vtable_final_func,
    },
};

ARCHI_PLUGIN_HELP_FUNC(archi_files_vtable_help_func)
{
    (void) topic;
    archi_print("\
This plugin provides contexts which are file descriptors of open files.\n\
\n\
Configuration options:\n\
    \"file\": archi_file_config_t -- the whole configuration structure\n\
or\n\
    \"pathname\": char[] -- path to file\n\
    \"mode\": char[] -- file opening mode\n\
");
    return 0;
}

static
ARCHI_LIST_ACT_FUNC(archi_files_vtable_init_func_config)
{
    archi_list_node_named_value_t *vnode = (archi_list_node_named_value_t*)node;
    archi_file_config_t *config = data;

    if (strcmp(vnode->base.name, ARCHI_PLUGIN_FILES_NAME) == 0) // whole configuration
    {
        if ((vnode->value.type != ARCHI_VALUE_DATA) || (vnode->value.ptr == NULL) ||
                (vnode->value.size != sizeof(*config)) || (vnode->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, vnode->value.ptr, vnode->value.size);
        return 0;
    }
    else if (strcmp(vnode->base.name, ARCHI_FILE_CONFIG_KEY_PATHNAME) == 0)
    {
        if ((vnode->value.type != ARCHI_VALUE_STRING) || (vnode->value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->pathname = vnode->value.ptr;
        return 0;
    }
    else if (strcmp(vnode->base.name, ARCHI_FILE_CONFIG_KEY_MODE) == 0)
    {
        if ((vnode->value.type != ARCHI_VALUE_STRING) || (vnode->value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->mode = vnode->value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_PLUGIN_INIT_FUNC(archi_files_vtable_init_func)
{
    if ((context == NULL) || (config == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_file_config_t file_config = {0};
    {
        archi_status_t code = archi_list_traverse((archi_list_t*)config, NULL, NULL,
                archi_files_vtable_init_func_config, &file_config, true, 0, NULL); // start from head
        if (code != 0)
            return code;
    }

    if ((file_config.pathname == NULL) || (file_config.mode == NULL))
        return ARCHI_ERROR_CONFIG;

    FILE *file = fopen(file_config.pathname, file_config.mode);
    if (file == NULL)
        return ARCHI_ERROR_FILE;

    *context = file;
    return 0;
}

ARCHI_PLUGIN_FINAL_FUNC(archi_files_vtable_final_func)
{
    fclose(context);
}

