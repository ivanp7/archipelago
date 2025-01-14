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
 * @brief Context interface of the plugin.
 */

#include "archi/plugin/threads/context.fun.h"
#include "archi/plugin/threads/config.typ.h"
#include "archi/plugin/threads/interface.fun.h"
#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <string.h> // for strcmp(), memcpy()

static
ARCHI_CONTAINER_ELEMENT_FUNC(archi_threads_context_init_config)
{
    if ((key == NULL) || (element == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_value_t *value = element;
    archi_threads_config_t *config = data;

    if (strcmp(key, ARCHI_THREADS_CONFIG_KEY) == 0)
    {
        if ((value->type != ARCHI_VALUE_DATA) || (value->ptr == NULL) ||
                (value->size != sizeof(*config)) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value->ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(key, ARCHI_THREADS_CONFIG_KEY_NUM_THREADS) == 0)
    {
        if ((value->type != ARCHI_VALUE_UINT) || (value->ptr == NULL) ||
                (value->size != sizeof(config->num_threads)) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->num_threads = *(size_t*)value->ptr;
        return 0;
    }
    else if (strcmp(key, ARCHI_THREADS_CONFIG_KEY_BUSY_WAIT) == 0)
    {
        switch (value->type)
        {
            case ARCHI_VALUE_FALSE:
                config->busy_wait = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->busy_wait = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(archi_threads_context_init)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_threads_config_t threads_config = {0};
    if (config.data != NULL)
    {
        code = archi_container_traverse(config, archi_threads_context_init_config, &threads_config);
        if (code != 0)
            return code;
    }

    struct archi_threads_context *threads_context = archi_threads_start(threads_config, &code);
    if (code != 0)
        return code;

    *context = threads_context;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_threads_context_final)
{
    archi_threads_stop(context);
}

const archi_context_interface_t archi_threads_context_interface = {
    .init_fn = archi_threads_context_init,
    .final_fn = archi_threads_context_final,
};

