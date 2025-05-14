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

            library_load_params.lazy = *(char*)params->value.ptr;
        }
        else if (strcmp("global", params->name) == 0)
        {
            if (param_global_set)
                continue;
            param_global_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            library_load_params.global = *(char*)params->value.ptr;
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
    if (slot.num_indices != 0)
        return ARCHI_STATUS_EMISUSE;

    archi_pointer_t symbol = context.private_value;
    context.private_value = (archi_pointer_t){0};

    symbol.ref_count = context.public_value.ref_count;

    symbol.ptr = archi_library_get_symbol(context.public_value.ptr, slot.name);
    if (symbol.ptr == NULL)
        return 1; // symbol not found

    *value = symbol;
    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_res_library_act)
{
    if (action.num_indices != 0)
        return ARCHI_STATUS_EMISUSE;

    archi_pointer_t attributes = {0};
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

            attributes.flags = *(uintptr_t*)params->value.ptr;

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

    if ((attributes.element.alignment & (attributes.element.alignment - 1)) != 0)
        return ARCHI_STATUS_EVALUE;

    context.private_value = attributes;
    return 0;
}

const archi_context_interface_t archi_context_res_library_interface = {
    .init_fn = archi_context_res_library_init,
    .final_fn = archi_context_res_library_final,
    .get_fn = archi_context_res_library_get,
    .act_fn = archi_context_res_library_act,
};

