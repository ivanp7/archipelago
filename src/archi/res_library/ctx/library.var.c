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

#include "archi/res_library/ctx/library.var.h"
#include "archi/res_library/api/handle.fun.h"
#include "archipelago/util/alloc.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()

struct archi_context_library_data {
    archi_pointer_t handle;

    archi_pointer_t default_attributes;
    archi_pointer_t current_attributes;
    char *current_symbol_name;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_library_init)
{
    archi_library_load_params_t library_load_params = {0};
    archi_library_load_params_t library_load_params_fields = {0};

    bool param_params_set = false;
    bool param_pathname_set = false;
    bool param_lazy_set = false;
    bool param_global_set = false;
    bool param_flags_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("params", params->name) == 0)
        {
            if (param_params_set)
                continue;
            param_params_set = true;

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

            library_load_params_fields.pathname = params->value.ptr;
        }
        else if (strcmp("lazy", params->name) == 0)
        {
            if (param_lazy_set)
                continue;
            param_lazy_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            library_load_params_fields.lazy = *(char*)params->value.ptr;
        }
        else if (strcmp("global", params->name) == 0)
        {
            if (param_global_set)
                continue;
            param_global_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            library_load_params_fields.global = *(char*)params->value.ptr;
        }
        else if (strcmp("flags", params->name) == 0)
        {
            if (param_flags_set)
                continue;
            param_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            library_load_params_fields.flags = *(int*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (param_pathname_set)
        library_load_params.pathname = library_load_params_fields.pathname;

    if (param_lazy_set)
        library_load_params.lazy = library_load_params_fields.lazy;

    if (param_global_set)
        library_load_params.global = library_load_params_fields.global;

    if (param_flags_set)
        library_load_params.flags = library_load_params_fields.flags;

    struct archi_context_library_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    void *handle = archi_library_load(library_load_params);
    if (handle == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    *context_data = (struct archi_context_library_data){
        .handle = {
            .ptr = handle,
            .element = {
                .num_of = 1,
            },
        },
        .default_attributes = {
            .element = {
                .num_of = 1,
            }
        },
        .current_attributes = {
            .element = {
                .num_of = 1,
            }
        },
    };

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_library_final)
{
    struct archi_context_library_data *context_data =
        (struct archi_context_library_data*)context;

    archi_library_unload(context_data->handle.ptr);
    free(context_data->current_symbol_name);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_library_get)
{
    if (slot.num_indices != 0)
        return ARCHI_STATUS_EMISUSE;

    struct archi_context_library_data *context_data =
        (struct archi_context_library_data*)context;

    archi_pointer_t symbol;

    if ((context_data->current_symbol_name != NULL) &&
            (strcmp(slot.name, context_data->current_symbol_name) == 0))
        symbol = context_data->current_attributes;
    else
        symbol = context_data->default_attributes;

    context_data->current_attributes = context_data->default_attributes;

    free(context_data->current_symbol_name);
    context_data->current_symbol_name = NULL;

    symbol.ptr = archi_library_get_symbol(context_data->handle.ptr, slot.name);
    if (symbol.ptr == NULL)
        return 1; // symbol not found

    symbol.ref_count = context_data->handle.ref_count;

    *value = symbol;
    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_library_act)
{
    if (action.num_indices != 0)
        return ARCHI_STATUS_EMISUSE;

    struct archi_context_library_data *context_data =
        (struct archi_context_library_data*)context;

    archi_pointer_t attributes = {
        .element = {
            .num_of = 1,
        },
    };
    bool flag_function = false;
    archi_array_layout_t layout_fields = {0};

    bool param_flag_function_set = false;
    bool param_flags_set = false;
    bool param_layout_set = false;
    bool param_num_elements_set = false;
    bool param_element_size_set = false;
    bool param_element_alignment_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("function", params->name) == 0)
        {
            if (param_flag_function_set)
                continue;
            param_flag_function_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            flag_function = *(char*)params->value.ptr;
        }
        else if (strcmp("flags", params->name) == 0)
        {
            if (param_flags_set)
                continue;
            param_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            attributes.flags = *(archi_pointer_flags_t*)params->value.ptr;

            if (attributes.flags > ARCHI_POINTER_USER_FLAGS_MASK)
                return ARCHI_STATUS_EVALUE;
        }
        else if (strcmp("layout", params->name) == 0)
        {
            if (param_layout_set)
                continue;
            param_layout_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            attributes.element = *(archi_array_layout_t*)params->value.ptr;
        }
        else if (strcmp("num_elements", params->name) == 0)
        {
            if (param_num_elements_set)
                continue;
            param_num_elements_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            layout_fields.num_of = *(size_t*)params->value.ptr;

            if (layout_fields.num_of == 0)
                return ARCHI_STATUS_EVALUE;
        }
        else if (strcmp("element_size", params->name) == 0)
        {
            if (param_element_size_set)
                continue;
            param_element_size_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            layout_fields.size = *(size_t*)params->value.ptr;
        }
        else if (strcmp("element_alignment", params->name) == 0)
        {
            if (param_element_alignment_set)
                continue;
            param_element_alignment_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            layout_fields.alignment = *(size_t*)params->value.ptr;

            if ((layout_fields.alignment & (layout_fields.alignment - 1)) != 0)
                return ARCHI_STATUS_EVALUE;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    attributes.flags |= flag_function ? ARCHI_POINTER_FLAG_FUNCTION : 0;

    if (param_num_elements_set)
        attributes.element.num_of = layout_fields.num_of;

    if (param_element_size_set)
        attributes.element.size = layout_fields.size;

    if (param_element_alignment_set)
        attributes.element.alignment = layout_fields.alignment;

    if (action.name[0] != '\0')
    {
        context_data->current_attributes = attributes;

        free(context_data->current_symbol_name);

        context_data->current_symbol_name = archi_copy_string(action.name);
        if (context_data->current_symbol_name == NULL)
            return ARCHI_STATUS_ENOMEMORY;
    }
    else
        context_data->default_attributes = attributes;

    return 0;
}

const archi_context_interface_t archi_context_library_interface = {
    .init_fn = archi_context_library_init,
    .final_fn = archi_context_library_final,
    .get_fn = archi_context_library_get,
    .act_fn = archi_context_library_act,
};

