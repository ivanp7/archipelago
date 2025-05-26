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
 * @brief Application context interface for environmental variables.
 */

#include "archi/builtin/ipc_env/context.var.h"
#include "archi/ipc/env/api.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), strlen()
#include <stdbool.h>

ARCHI_CONTEXT_INIT_FUNC(archi_context_ipc_env_init)
{
    const char *name = NULL;

    bool param_name_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("name", params->name) == 0)
        {
            if (param_name_set)
                continue;
            param_name_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            name = params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;
    char *var = archi_env_get(name, &code);

    if (var == NULL)
    {
        free(context_data);
        return code;
    }

    *context_data = (archi_pointer_t){
        .ptr = var,
        .element = {
            .num_of = strlen(var) + 1,
            .size = 1,
            .alignment = 1,
        },
    };

    *context = context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_ipc_env_final)
{
    free(context->ptr);
    free(context);
}

const archi_context_interface_t archi_context_ipc_env_interface = {
    .init_fn = archi_context_ipc_env_init,
    .final_fn = archi_context_ipc_env_final,
};

