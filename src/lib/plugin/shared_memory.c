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
#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <string.h> // for strcmp(), memcpy()

static
ARCHI_CONTAINER_ELEMENT_FUNC(archi_shared_memory_context_init_config)
{
    if ((key == NULL) || (element == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_value_t *value = element;
    archi_shared_memory_config_t *config = data;

    if (strcmp(key, ARCHI_SHARED_MEMORY_CONFIG_KEY) == 0)
    {
        if ((value->type != ARCHI_VALUE_DATA) || (value->ptr == NULL) ||
                (value->size != sizeof(*config)) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value->ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(key, ARCHI_SHARED_MEMORY_CONFIG_KEY_PATHNAME) == 0)
    {
        if ((value->type != ARCHI_VALUE_STRING) || (value->ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->pathname = value->ptr;
        return 0;
    }
    else if (strcmp(key, ARCHI_SHARED_MEMORY_CONFIG_KEY_PROJECT_ID) == 0)
    {
        if ((value->type != ARCHI_VALUE_UINT) || (value->ptr == NULL) ||
                (value->size != 1) || (value->num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->proj_id = *(unsigned char*)value->ptr;
        return 0;
    }
    else if (strcmp(key, ARCHI_SHARED_MEMORY_CONFIG_KEY_WRITABLE) == 0)
    {
        switch (value->type)
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
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(archi_shared_memory_context_init)
{
    if (context == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    archi_shared_memory_config_t shared_memory_config = {0};
    if (config.data != NULL)
    {
        code = archi_container_traverse(config, archi_shared_memory_context_init_config, &shared_memory_config);
        if (code != 0)
            return code;
    }

    if ((shared_memory_config.pathname == NULL) || (shared_memory_config.proj_id == 0))
        return ARCHI_ERROR_CONFIG;

    void **shmaddr = archi_shared_memory_attach(shared_memory_config.pathname,
            shared_memory_config.proj_id, shared_memory_config.writable);
    if (shmaddr == NULL)
        return ARCHI_ERROR_ATTACH;

    *context = shmaddr;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_shared_memory_context_final)
{
    archi_shared_memory_detach(context);
}

const archi_context_interface_t archi_shared_memory_context_interface = {
    .init_fn = archi_shared_memory_context_init,
    .final_fn = archi_shared_memory_context_final,
};

