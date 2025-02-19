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
 * @brief Built-in plugin for opening files.
 */

#include "archi/plugin/files/interface.fun.h"
#include "archi/plugin/files/config.typ.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <stdio.h> // for FILE, fopen(), fclose()
#include <string.h> // for strcmp(), memcpy()

static
ARCHI_LIST_ACT_FUNC(archi_plugin_file_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    archi_file_config_t *config = data;

    if (strcmp(config_node->base.name, ARCHI_FILE_CONFIG_KEY) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_DATA) || (config_node->value.ptr == NULL) ||
                (config_node->value.size != sizeof(*config)) || (config_node->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, config_node->value.ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_CONFIG_KEY_PATHNAME) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_STRING) || (config_node->value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->pathname = config_node->value.ptr;
        return 0;
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_CONFIG_KEY_MODE) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_STRING) || (config_node->value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->mode = config_node->value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_file_context_init)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_file_config_t file_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                archi_plugin_file_context_init_config, &file_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    if ((file_config.pathname == NULL) || (file_config.mode == NULL))
        return ARCHI_ERROR_CONFIG;

    FILE *file = fopen(file_config.pathname, file_config.mode);
    if (file == NULL)
        return ARCHI_ERROR_RESOURCE;

    *context = file;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_file_context_final)
{
    fclose(context);
}

const archi_context_interface_t archi_plugin_file_context_interface = {
    .init_fn = archi_plugin_file_context_init,
    .final_fn = archi_plugin_file_context_final,
};

