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
 * @brief Built-in plugin for loading shared libraries.
 */

#include "archi/plugin/shared_libraries.h"
#include "archi/util/os/library.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), memcpy()

struct archi_plugin_shared_library_context_metadata {
    bool symbol_type_function;
};

static
ARCHI_LIST_ACT_FUNC(archi_plugin_shared_library_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    archi_library_load_config_t *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, ARCHI_LIBRARY_LOAD_CONFIG_KEY) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) ||
                (value.size != sizeof(*config)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value.ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(name, ARCHI_LIBRARY_LOAD_CONFIG_KEY_PATHNAME) == 0)
    {
        if ((value.type != ARCHI_VALUE_STRING) || (value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->pathname = value.ptr;
        return 0;
    }
    else if (strcmp(name, ARCHI_LIBRARY_LOAD_CONFIG_KEY_LAZY) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->lazy = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->lazy = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(name, ARCHI_LIBRARY_LOAD_CONFIG_KEY_GLOBAL) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->global = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->global = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(name, ARCHI_LIBRARY_LOAD_CONFIG_KEY_FLAGS) == 0)
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

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_shared_library_context_init)
{
    archi_library_load_config_t shared_library_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        archi_status_t code = archi_list_traverse(&config_list, NULL, NULL,
                archi_plugin_shared_library_context_init_config, &shared_library_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    if (shared_library_config.pathname == NULL)
        return ARCHI_ERROR_CONFIG;

    struct archi_plugin_shared_library_context_metadata *meta = malloc(sizeof(*meta));
    if (meta == NULL)
        return ARCHI_ERROR_ALLOC;

    *meta = (struct archi_plugin_shared_library_context_metadata){0};

    void *handle = archi_library_load(shared_library_config);
    if (handle == NULL)
    {
        free(meta);
        return ARCHI_ERROR_LOAD;
    }

    *context = handle;
    *metadata = meta;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_shared_library_context_final)
{
    archi_library_unload(context);
    free(metadata);
}

ARCHI_CONTEXT_GET_FUNC(archi_plugin_shared_library_context_get)
{
    const struct archi_plugin_shared_library_context_metadata *meta = metadata;

    void *symbol = archi_library_get_symbol(context, slot);
    if (symbol == NULL)
        return ARCHI_ERROR_SYMBOL;

    *value = (archi_value_t){
        .ptr = symbol,
        .num_of = 1,
        .type = meta->symbol_type_function ? ARCHI_VALUE_FUNCTION : ARCHI_VALUE_DATA,
    };

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_plugin_shared_library_context_act)
{
    (void) context;
    (void) params;

    struct archi_plugin_shared_library_context_metadata *meta = metadata;

    if (strcmp(action, ARCHI_PLUGIN_SHARED_LIBRARY_ACTION_SYMTYPE_FUNC) == 0)
        meta->symbol_type_function = true;
    else if (strcmp(action, ARCHI_PLUGIN_SHARED_LIBRARY_ACTION_SYMTYPE_DATA) == 0)
        meta->symbol_type_function = false;
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

const archi_context_interface_t archi_plugin_shared_library_context_interface = {
    .init_fn = archi_plugin_shared_library_context_init,
    .final_fn = archi_plugin_shared_library_context_final,
    .get_fn = archi_plugin_shared_library_context_get,
    .act_fn = archi_plugin_shared_library_context_act,
};

