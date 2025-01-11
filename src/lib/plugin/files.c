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

#include "archi/plugin/files/context.fun.h"
#include "archi/plugin/files/config.typ.h"
#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <stdio.h> // for FILE, fopen(), fclose()
#include <string.h> // for strcmp(), memcpy()

static
ARCHI_CONTAINER_ELEMENT_FUNC(archi_file_context_init_config)
{
    if ((key == NULL) || (element == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_value_t *value = element;
    archi_file_config_t *config = data;

    if (strcmp(key, ARCHI_FILE_CONFIG_KEY) == 0)
    {
        if ((value->type != ARCHI_VALUE_DATA) || (value->ptr == NULL) ||
                (value->size != sizeof(*config)) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value->ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(key, ARCHI_FILE_CONFIG_KEY_PATHNAME) == 0)
    {
        if ((value->type != ARCHI_VALUE_STRING) || (value->ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->pathname = value->ptr;
        return 0;
    }
    else if (strcmp(key, ARCHI_FILE_CONFIG_KEY_MODE) == 0)
    {
        if ((value->type != ARCHI_VALUE_STRING) || (value->ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->mode = value->ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(archi_file_context_init)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_file_config_t file_config = {0};
    if (config.data != NULL)
    {
        code = archi_container_traverse(config, archi_file_context_init_config, &file_config);
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

ARCHI_CONTEXT_FINAL_FUNC(archi_file_context_final)
{
    fclose(context);
}

const archi_context_interface_t archi_file_context_interface = {
    .init_fn = archi_file_context_init,
    .final_fn = archi_file_context_final,
};

