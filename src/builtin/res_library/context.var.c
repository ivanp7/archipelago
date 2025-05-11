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
 * @brief Application context interface for shared libraries.
 */

#include "archi/builtin/res_library/context.var.h"
#include "archi/res/library/api.fun.h"

#include <string.h> // for strcmp()

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_library_init)
{
    archi_library_load_params_t library_load_params = {0};

    bool param_struct_set = false;
    bool param_pathname_set = false;
    bool param_lazy_set = false;
    bool param_global_set = false;
    bool param_flags_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("params", params->name) == 0)
        {
            if (param_struct_set)
                continue;
            param_struct_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            library_load_params = *(archi_library_load_params_t*)params->value.ptr;
        }
        else if (strcmp("pathname", params->name) == 0)
        {
            if (param_pathname_set)
                continue;
            param_pathname_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            library_load_params.pathname = params->value.ptr;
        }
        else if (strcmp("lazy", params->name) == 0)
        {
            if (param_lazy_set)
                continue;
            param_lazy_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            library_load_params.lazy = *(bool*)params->value.ptr;
        }
        else if (strcmp("global", params->name) == 0)
        {
            if (param_global_set)
                continue;
            param_global_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            library_load_params.global = *(bool*)params->value.ptr;
        }
        else if (strcmp("flags", params->name) == 0)
        {
            if (param_flags_set)
                continue;
            param_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            library_load_params.flags = *(int*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    void *handle = archi_library_load(library_load_params);
    if (handle == NULL)
        return ARCHI_STATUS_ERESOURCE;

    context->public_value = (archi_pointer_t){
        .ptr = handle,
        .element = {
            .num_of = 1,
        },
    };
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_library_final)
{
    archi_library_unload(context.public_value.ptr);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_library_get)
{
    if (slot.num_indices > 5)
        return ARCHI_STATUS_EMISUSE;

    archi_pointer_t symbol = {.ref_count = context.public_value.ref_count};

    if (slot.num_indices >= 1)
    {
        if (slot.index[0] > 1)
            return ARCHI_STATUS_EMISUSE;

        symbol.flags |= slot.index[0] ? ARCHI_POINTER_FLAG_FUNCTION : 0;
    }

    if (slot.num_indices >= 2)
    {
        if (slot.index[1] > ARCHI_POINTER_USER_FLAGS_MASK)
            return ARCHI_STATUS_EMISUSE;

        symbol.flags |= slot.index[1];
    }

    if (slot.num_indices >= 3)
    {
        if (slot.index[2] == 0)
            return ARCHI_STATUS_EMISUSE;

        symbol.element.num_of = slot.index[2];
    }

    if (slot.num_indices >= 4)
        symbol.element.size = slot.index[3];

    if (slot.num_indices >= 5)
    {
        if ((slot.index[4] & (slot.index[4] - 1)) != 0)
            return ARCHI_STATUS_EMISUSE;

        symbol.element.alignment = slot.index[4];
    }

    symbol.ptr = archi_library_get_symbol(context.public_value.ptr, slot.name);
    if (symbol.ptr == NULL)
        return 1; // symbol not found

    *value = symbol;
    return 0;
}

const archi_context_interface_t archi_context_res_library_interface = {
    .init_fn = archi_context_res_library_init,
    .final_fn = archi_context_res_library_final,
    .get_fn = archi_context_res_library_get,
};

