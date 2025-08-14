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

#include "archi/ipc_env/ctx/envvar.var.h"
#include "archi/ipc_env/api/envvar.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), strlen()
#include <stdbool.h>

static
ARCHI_DESTRUCTOR_FUNC(archi_context_ipc_env_destructor)
{
    free(data);
}

ARCHI_CONTEXT_INIT_FUNC(archi_context_ipc_env_init)
{
    const char *name = NULL;
    archi_pointer_t default_value = {0};

    bool param_name_set = false;
    bool param_default_value_set = false;

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
        else if (strcmp("default_value", params->name) == 0)
        {
            if (param_default_value_set)
                continue;
            param_default_value_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            default_value = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_status_t code;
    char *var = archi_env_get(name, &code);

    if (var != NULL)
    {
        archi_reference_count_t ref_count =
            archi_reference_count_alloc(archi_context_ipc_env_destructor, var);
        if (ref_count == NULL)
        {
            free(var);
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        *context_data = (archi_pointer_t){
            .ptr = var,
            .ref_count = ref_count,
            .element = {
                .num_of = strlen(var) + 1,
                .size = 1,
                .alignment = 1,
            },
        };
    }
    else if ((code == 1) && param_default_value_set)
    {
        *context_data = default_value;
        archi_reference_count_increment(default_value.ref_count);
    }
    else
    {
        free(context_data);
        return code;
    }

    *context = context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_ipc_env_final)
{
    archi_reference_count_decrement(context->ref_count);
    free(context);
}

const archi_context_interface_t archi_context_ipc_env_interface = {
    .init_fn = archi_context_ipc_env_init,
    .final_fn = archi_context_ipc_env_final,
};

