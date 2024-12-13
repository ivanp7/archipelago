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
 * @brief Built-in plugin for loading shared memory.
 */

#include "archi/plugin/shared_memory/vtable.fun.h"
#include "archi/plugin/shared_memory/vtable.def.h"
#include "archi/plugin/shared_memory/vtable.var.h"
#include "archi/plugin/shared_memory/interface.typ.h"
#include "archi/util/os/shm.fun.h"
#include "archi/app/plugin.def.h"
#include "archi/app/version.def.h"
#include "archi/util/list.fun.h"
#include "archi/util/list.def.h"
#include "archi/util/value.typ.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"

#include <string.h> // for strcmp()

const archi_plugin_vtable_t archi_vtable_shared_memory = {
    .format = {.magic = ARCHI_API_MAGIC, .version = ARCHI_API_VERSION},
    .info = {.name = ARCHI_PLUGIN_SHARED_MEMORY_NAME,
        .description = "Operations with shared memory.",
        .help_fn = archi_shared_memory_vtable_help_func,
    },
    .func = {
        .init_fn = archi_shared_memory_vtable_init_func,
        .final_fn = archi_shared_memory_vtable_final_func,
    },
};

ARCHI_PLUGIN_HELP_FUNC(archi_shared_memory_vtable_help_func)
{
    (void) topic;
    archi_print("\
This plugin provides contexts which are addresses of loaded shared memory.\n\
The address is to be casted to type void**, which is an array of pointers\n\
to data structures in the shared memory. The first element (index 0) is always\n\
equal to the shared memory address itself.\n\
\n\
Configuration options:\n\
    \"shared_memory\": archi_shared_memory_config_t -- the whole configuration structure\n\
or\n\
    \"pathname\": char[] -- shared memory key pathname\n\
    \"proj_id\": int -- shared memory key project identifier (1-255)\n\
    \"writable\": bool -- whether attached memory is writable\n\
");
    return 0;
}

static
ARCHI_LIST_ACT_FUNC(archi_shared_memory_vtable_init_func_config)
{
    archi_list_node_named_value_t *vnode = (archi_list_node_named_value_t*)node;
    archi_shared_memory_config_t *config = data;

    if (strcmp(vnode->base.name, ARCHI_PLUGIN_SHARED_MEMORY_NAME) == 0) // whole configuration
    {
        if ((vnode->value.type != ARCHI_VALUE_DATA) || (vnode->value.ptr == NULL) ||
                (vnode->value.size != sizeof(*config)) || (vnode->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, vnode->value.ptr, vnode->value.size);
        return 0;
    }
    else if (strcmp(vnode->base.name, ARCHI_SHARED_MEMORY_PATHNAME) == 0)
    {
        if ((vnode->value.type != ARCHI_VALUE_STRING) || (vnode->value.ptr == NULL))
            return ARCHI_ERROR_CONFIG;

        config->pathname = vnode->value.ptr;
        return 0;
    }
    else if (strcmp(vnode->base.name, ARCHI_SHARED_MEMORY_PROJECT_ID) == 0)
    {
        if ((vnode->value.type != ARCHI_VALUE_UINT) || (vnode->value.ptr == NULL) ||
                (vnode->value.size != 1) || (vnode->value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->proj_id = *(unsigned char*)vnode->value.ptr;
        return 0;
    }
    else if (strcmp(vnode->base.name, ARCHI_SHARED_MEMORY_WRITABLE) == 0)
    {
        switch (vnode->value.type)
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

ARCHI_PLUGIN_INIT_FUNC(archi_shared_memory_vtable_init_func)
{
    if ((context == NULL) || (config == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_shared_memory_config_t shared_memory_config = {0};
    {
        archi_status_t code = archi_list_traverse((archi_list_t*)config, NULL, NULL,
                archi_shared_memory_vtable_init_func_config, &shared_memory_config, true, 0, NULL); // start from head
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

ARCHI_PLUGIN_FINAL_FUNC(archi_shared_memory_vtable_final_func)
{
    archi_shared_memory_detach(context);
}

