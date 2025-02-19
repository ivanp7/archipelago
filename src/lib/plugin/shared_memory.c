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
 * @brief Built-in plugin for attaching shared memory.
 */

#include "archi/plugin/shared_memory/interface.fun.h"
#include "archi/plugin/shared_memory/config.typ.h"
#include "archi/util/os/shm.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <string.h> // for strcmp(), memcpy()

static
ARCHI_LIST_ACT_FUNC(archi_plugin_shared_memory_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    archi_shared_memory_config_t *config = data;

    if (strcmp(config_node->base.name, ARCHI_SHARED_MEMORY_CONFIG_KEY) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_DATA) || (config_node->value.ptr == NULL) ||
                (config_node->value.size != sizeof(*config)) || (config_node->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, config_node->value.ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(config_node->base.name, ARCHI_SHARED_MEMORY_CONFIG_KEY_FILE) == 0)
    {
        if ((config_node->value.type != ARCHI_VALUE_STRING) || (config_node->value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->file = config_node->value.ptr;
        return 0;
    }
    else if (strcmp(config_node->base.name, ARCHI_SHARED_MEMORY_CONFIG_KEY_READABLE) == 0)
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
    else if (strcmp(config_node->base.name, ARCHI_SHARED_MEMORY_CONFIG_KEY_WRITABLE) == 0)
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
    else if (strcmp(config_node->base.name, ARCHI_SHARED_MEMORY_CONFIG_KEY_SHARED) == 0)
    {
        switch (config_node->value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->shared = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->shared = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(config_node->base.name, ARCHI_SHARED_MEMORY_CONFIG_KEY_FLAGS) == 0)
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

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_shared_memory_context_init)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_shared_memory_config_t shared_memory_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                archi_plugin_shared_memory_context_init_config, &shared_memory_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    if (shared_memory_config.file == NULL)
        return ARCHI_ERROR_CONFIG;

    int fd = archi_shm_open_file(shared_memory_config.file,
            shared_memory_config.readable, shared_memory_config.writable);
    if (fd == -1)
        return ARCHI_ERROR_MAP;

    archi_shm_header_t *shm = archi_shm_map(fd, shared_memory_config.readable,
            shared_memory_config.writable, shared_memory_config.shared, shared_memory_config.flags);
    if (shm == NULL)
    {
        archi_shm_close(fd);
        return ARCHI_ERROR_MAP;
    }

    archi_shm_close(fd);

    *context = shm;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_shared_memory_context_final)
{
    archi_shm_unmap(context);
}

const archi_context_interface_t archi_plugin_shared_memory_context_interface = {
    .init_fn = archi_plugin_shared_memory_context_init,
    .final_fn = archi_plugin_shared_memory_context_final,
};

