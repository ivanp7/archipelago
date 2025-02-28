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
 * @brief File descriptors and mapped memory.
 */

#include "archi/plugin/files.h"
#include "archi/util/os/file.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), memcpy()

static
ARCHI_LIST_ACT_FUNC(archi_plugin_file_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    archi_file_open_config_t *config = data;

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
    else if (strcmp(config_node->base.name, ARCHI_FILE_CONFIG_KEY_READABLE) == 0)
    {
        switch (config_node->value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->readable = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->readable = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_CONFIG_KEY_WRITABLE) == 0)
    {
        switch (config_node->value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->writable = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->writable = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_CONFIG_KEY_NONBLOCK) == 0)
    {
        switch (config_node->value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->nonblock = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->nonblock = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_CONFIG_KEY_FLAGS) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_SINT) || (config_node->value.ptr == NULL) ||
                (config_node->value.size != sizeof(int)) || (config_node->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->flags = *(int*)config_node->value.ptr;
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

    archi_file_open_config_t file_open_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                archi_plugin_file_context_init_config, &file_open_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    if (file_open_config.pathname == NULL)
        return ARCHI_ERROR_CONFIG;

    int *fd = malloc(sizeof(*fd));
    if (fd == NULL)
        return ARCHI_ERROR_ALLOC;

    *fd = archi_file_open(file_open_config);

    if (*fd == -1)
    {
        free(fd);
        return ARCHI_ERROR_OPEN;
    }

    *context = fd;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_file_context_final)
{
    int *fd = context;
    if (fd != NULL)
    {
        archi_file_close(*fd);
        free(fd);
    }
}

const archi_context_interface_t archi_plugin_file_context_interface = {
    .init_fn = archi_plugin_file_context_init,
    .final_fn = archi_plugin_file_context_final,
};

/*****************************************************************************/

struct archi_plugin_file_map_config {
    const char *pathname;
    archi_file_map_config_t param;
};

static
ARCHI_LIST_ACT_FUNC(archi_plugin_file_map_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    struct archi_plugin_file_map_config *config = data;

    if (strcmp(config_node->base.name, ARCHI_PLUGIN_FILE_MAP_CONFIG_KEY_PATHNAME) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_STRING) || (config_node->value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->pathname = config_node->value.ptr;
        return 0;
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_MAP_CONFIG_KEY) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_DATA) || (config_node->value.ptr == NULL) ||
                (config_node->value.size != sizeof(config->param)) || (config_node->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(&config->param, config_node->value.ptr, sizeof(config->param));
        return 0;
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_MAP_CONFIG_KEY_SIZE) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_UINT) || (config_node->value.ptr == NULL) ||
                (config_node->value.size != sizeof(size_t)) || (config_node->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->param.size = *(size_t*)config_node->value.ptr;
        return 0;
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_MAP_CONFIG_KEY_OFFSET) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_UINT) || (config_node->value.ptr == NULL) ||
                (config_node->value.size != sizeof(size_t)) || (config_node->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->param.offset = *(size_t*)config_node->value.ptr;
        return 0;
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_MAP_CONFIG_KEY_READABLE) == 0)
    {
        switch (config_node->value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->param.readable = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->param.readable = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_MAP_CONFIG_KEY_WRITABLE) == 0)
    {
        switch (config_node->value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->param.writable = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->param.writable = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_MAP_CONFIG_KEY_SHARED) == 0)
    {
        switch (config_node->value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->param.shared = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->param.shared = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(config_node->base.name, ARCHI_FILE_MAP_CONFIG_KEY_FLAGS) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_SINT) || (config_node->value.ptr == NULL) ||
                (config_node->value.size != sizeof(int)) || (config_node->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->param.flags = *(int*)config_node->value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_file_map_context_init)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    struct archi_plugin_file_map_config file_map_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                archi_plugin_file_map_context_init_config, &file_map_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    if (file_map_config.pathname == NULL)
        return ARCHI_ERROR_CONFIG;

    archi_file_open_config_t file_open_config = {
        .pathname = file_map_config.pathname,
        .readable = file_map_config.param.readable,
        .writable = file_map_config.param.writable,
        .nonblock = true,
    };

    int fd = archi_file_open(file_open_config);

    if (fd == -1)
        return ARCHI_ERROR_OPEN;

    archi_mmap_header_t *mm = archi_file_map(fd, file_map_config.param);

    if (mm == NULL)
    {
        archi_file_close(fd);
        return ARCHI_ERROR_MAP;
    }

    archi_file_close(fd);

    *context = mm;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_file_map_context_final)
{
    archi_file_unmap(context);
}

const archi_context_interface_t archi_plugin_file_map_context_interface = {
    .init_fn = archi_plugin_file_map_context_init,
    .final_fn = archi_plugin_file_map_context_final,
};

