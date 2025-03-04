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

struct archi_plugin_file_context {
    int fd;   ///< File descriptor.

    void *mm; ///< Mapped memory.
    size_t mm_size; ///< Mapped memory size.
};

int
archi_plugin_file_context_descriptor(
        struct archi_plugin_file_context *context)
{
    if (context == NULL)
        return -1;

    return context->fd;
}

void*
archi_plugin_file_context_mapped_memory(
        struct archi_plugin_file_context *context,

        size_t *memory_size)
{
    if (context == NULL)
        return NULL;

    if (memory_size != NULL)
        *memory_size = context->mm_size;

    return context->mm;
}

static
ARCHI_LIST_ACT_FUNC(archi_plugin_file_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    archi_file_open_config_t *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, ARCHI_FILE_CONFIG_KEY) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) ||
                (value.size != sizeof(*config)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value.ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(name, ARCHI_FILE_CONFIG_KEY_PATHNAME) == 0)
    {
        if ((value.type != ARCHI_VALUE_STRING) || (value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->pathname = value.ptr;
        return 0;
    }
    else if (strcmp(name, ARCHI_FILE_CONFIG_KEY_READABLE) == 0)
    {
        switch (value.type)
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
    else if (strcmp(name, ARCHI_FILE_CONFIG_KEY_WRITABLE) == 0)
    {
        switch (value.type)
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
    else if (strcmp(name, ARCHI_FILE_CONFIG_KEY_NONBLOCK) == 0)
    {
        switch (value.type)
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
    else if (strcmp(name, ARCHI_FILE_CONFIG_KEY_FLAGS) == 0)
    {
        if ((value.type != ARCHI_VALUE_SINT) || (value.ptr == NULL) ||
                (value.size != sizeof(int)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->flags = *(int*)value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_file_context_init)
{
    (void) metadata;

    archi_file_open_config_t file_open_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        archi_status_t code = archi_list_traverse(&config_list, NULL, NULL,
                archi_plugin_file_context_init_config, &file_open_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    if (file_open_config.pathname == NULL)
        return ARCHI_ERROR_CONFIG;

    struct archi_plugin_file_context *file_context = malloc(sizeof(*file_context));
    if (file_context == NULL)
        return ARCHI_ERROR_ALLOC;

    *file_context = (struct archi_plugin_file_context){.fd = archi_file_open(file_open_config)};

    if (file_context->fd == -1)
    {
        free(file_context);
        return ARCHI_ERROR_OPEN;
    }

    *context = file_context;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_file_context_final)
{
    (void) metadata;

    struct archi_plugin_file_context *file_context = context;

    if (file_context->mm != NULL)
        archi_file_unmap(file_context->mm, file_context->mm_size);

    if (file_context->fd != -1)
        archi_file_close(file_context->fd);

    free(file_context);
}

ARCHI_CONTEXT_GET_FUNC(archi_plugin_file_context_get)
{
    (void) metadata;

    struct archi_plugin_file_context *file_context = context;

    if (strcmp(slot, ARCHI_PLUGIN_FILE_SLOT_FILE_DESCRIPTOR) == 0)
    {
        *value = (archi_value_t){
            .ptr = &file_context->fd,
            .size = sizeof(file_context->fd),
            .num_of = 1,
            .type = ARCHI_VALUE_SINT,
        };
    }
    else if (strcmp(slot, ARCHI_PLUGIN_FILE_SLOT_MAPPED_MEMORY) == 0)
    {
        *value = (archi_value_t){
            .ptr = file_context->mm,
            .size = file_context->mm_size,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

struct archi_plugin_file_context_act_params {
    archi_file_map_config_t map;
    bool close_fd;
};

static
ARCHI_LIST_ACT_FUNC(archi_plugin_file_context_act_params)
{
    (void) position;

    archi_list_node_named_value_t *params_node = (archi_list_node_named_value_t*)node;
    struct archi_plugin_file_context_act_params *params = data;

    const char *name = params_node->base.name;
    archi_value_t value = params_node->value;

    if (strcmp(name, ARCHI_FILE_MAP_PARAM_KEY) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) ||
                (value.size != sizeof(params->map)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(&params->map, value.ptr, sizeof(params->map));
        return 0;
    }
    else if (strcmp(name, ARCHI_FILE_MAP_PARAM_KEY_SIZE) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(size_t)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        params->map.size = *(size_t*)value.ptr;
        return 0;
    }
    else if (strcmp(name, ARCHI_FILE_MAP_PARAM_KEY_OFFSET) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(size_t)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        params->map.offset = *(size_t*)value.ptr;
        return 0;
    }
    else if (strcmp(name, ARCHI_FILE_MAP_PARAM_KEY_HAS_HEADER) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                params->map.has_header = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                params->map.has_header = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(name, ARCHI_FILE_MAP_PARAM_KEY_READABLE) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                params->map.readable = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                params->map.readable = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(name, ARCHI_FILE_MAP_PARAM_KEY_WRITABLE) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                params->map.writable = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                params->map.writable = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(name, ARCHI_FILE_MAP_PARAM_KEY_SHARED) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                params->map.shared = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                params->map.shared = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(name, ARCHI_FILE_MAP_PARAM_KEY_FLAGS) == 0)
    {
        if ((value.type != ARCHI_VALUE_SINT) || (value.ptr == NULL) ||
                (value.size != sizeof(int)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        params->map.flags = *(int*)value.ptr;
        return 0;
    }
    else if (strcmp(name, ARCHI_PLUGIN_FILE_ACTION_MAP_PARAM_CLOSE) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                params->close_fd = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                params->close_fd = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_ACT_FUNC(archi_plugin_file_context_act)
{
    (void) metadata;

    struct archi_plugin_file_context *file_context = context;

    if (strcmp(action, ARCHI_PLUGIN_FILE_ACTION_MAP) == 0)
    {
        if (file_context->mm != NULL)
            return ARCHI_ERROR_MISUSE;

        struct archi_plugin_file_context_act_params file_map_params = {0};
        if (params != NULL)
        {
            archi_list_t params_list = {.head = (archi_list_node_t*)params};
            archi_status_t code = archi_list_traverse(&params_list, NULL, NULL,
                    archi_plugin_file_context_act_params, &file_map_params, true, 0, NULL);
            if (code != 0)
                return code;
        }

        file_context->mm = archi_file_map(file_context->fd, file_map_params.map,
                &file_context->mm_size);

        if (file_context->mm == NULL)
            return ARCHI_ERROR_MAP;

        if (file_map_params.close_fd)
        {
            archi_file_close(file_context->fd);
            file_context->fd = -1;
        }
    }
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

const archi_context_interface_t archi_plugin_file_context_interface = {
    .init_fn = archi_plugin_file_context_init,
    .final_fn = archi_plugin_file_context_final,
    .get_fn = archi_plugin_file_context_get,
    .act_fn = archi_plugin_file_context_act,
};

