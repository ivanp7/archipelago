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
 * @brief Application context interface for files.
 */

#include "archi/builtin/res_file/context.var.h"
#include "archi/res/file/api.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_file_init)
{
    archi_file_open_params_t file_open_params = {0};

    bool param_struct_set = false;
    bool param_pathname_set = false;
    bool param_readable_set = false;
    bool param_writable_set = false;
    bool param_nonblock_set = false;
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

            file_open_params = *(archi_file_open_params_t*)params->value.ptr;
        }
        else if (strcmp("pathname", params->name) == 0)
        {
            if (param_pathname_set)
                continue;
            param_pathname_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params.pathname = params->value.ptr;
        }
        else if (strcmp("readable", params->name) == 0)
        {
            if (param_readable_set)
                continue;
            param_readable_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params.readable = *(char*)params->value.ptr;
        }
        else if (strcmp("writable", params->name) == 0)
        {
            if (param_writable_set)
                continue;
            param_writable_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params.writable = *(char*)params->value.ptr;
        }
        else if (strcmp("nonblock", params->name) == 0)
        {
            if (param_nonblock_set)
                continue;
            param_nonblock_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params.nonblock = *(char*)params->value.ptr;
        }
        else if (strcmp("flags", params->name) == 0)
        {
            if (param_flags_set)
                continue;
            param_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params.flags = *(int*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    int *fd = malloc(sizeof(*fd));
    if (fd == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *fd = archi_file_open(file_open_params);

    if (*fd < 0)
    {
        free(fd);
        return ARCHI_STATUS_ERESOURCE;
    }

    context->private_value = (archi_pointer_t){
        .ptr = fd,
        .element = {
            .num_of = 1,
            .size = sizeof(*fd),
            .alignment = alignof(int),
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_file_final)
{
    archi_file_unmap(context.public_value.ptr, context.public_value.element.num_of);

    int *fd = context.private_value.ptr;
    if (fd != NULL)
    {
        archi_file_close(*fd);
        free(fd);
    }
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_file_get)
{
    if (strcmp("fd", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = context.private_value.ptr,
            .ref_count = context.public_value.ref_count,
            .element = context.private_value.element,
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_res_file_act)
{
    if (strcmp("map", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (context.public_value.ptr != NULL)
            return ARCHI_STATUS_EMISUSE;

        archi_file_map_params_t file_map_params = {0};
        bool close_fd = false;

        bool param_struct_set = false;
        bool param_size_set = false;
        bool param_offset_set = false;
        bool param_has_header_set = false;
        bool param_readable_set = false;
        bool param_writable_set = false;
        bool param_shared_set = false;
        bool param_flags_set = false;
        bool param_close_fd_set = false;

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

                file_map_params = *(archi_file_map_params_t*)params->value.ptr;
            }
            else if (strcmp("size", params->name) == 0)
            {
                if (param_size_set)
                    continue;
                param_size_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params.size = *(size_t*)params->value.ptr;
            }
            else if (strcmp("offset", params->name) == 0)
            {
                if (param_offset_set)
                    continue;
                param_offset_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params.offset = *(size_t*)params->value.ptr;
            }
            else if (strcmp("has_header", params->name) == 0)
            {
                if (param_has_header_set)
                    continue;
                param_has_header_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params.has_header = *(char*)params->value.ptr;
            }
            else if (strcmp("readable", params->name) == 0)
            {
                if (param_readable_set)
                    continue;
                param_readable_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params.readable = *(char*)params->value.ptr;
            }
            else if (strcmp("writable", params->name) == 0)
            {
                if (param_writable_set)
                    continue;
                param_writable_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params.writable = *(char*)params->value.ptr;
            }
            else if (strcmp("shared", params->name) == 0)
            {
                if (param_shared_set)
                    continue;
                param_shared_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params.shared = *(char*)params->value.ptr;
            }
            else if (strcmp("flags", params->name) == 0)
            {
                if (param_flags_set)
                    continue;
                param_flags_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params.flags = *(int*)params->value.ptr;
            }
            else if (strcmp("close_fd", params->name) == 0)
            {
                if (param_close_fd_set)
                    continue;
                param_close_fd_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                close_fd = *(char*)params->value.ptr;
            }
            else
                return ARCHI_STATUS_EKEY;
        }

        int *fd = context.private_value.ptr;

        size_t mm_size = 0;
        void *mm = archi_file_map(*fd, file_map_params, &mm_size);

        if (mm == NULL)
            return ARCHI_STATUS_ERESOURCE;

        archi_reference_count_t ref_count = context.public_value.ref_count;

        context.public_value = (archi_pointer_t){
            .ptr = mm,
            .ref_count = ref_count,
            .flags = file_map_params.writable ? ARCHI_POINTER_FLAG_WRITABLE : 0,
            .element = {
                .num_of = mm_size,
                .size = 1,
            },
        };

        if (close_fd)
        {
            archi_file_close(*fd);
            free(fd);
            context.private_value = (archi_pointer_t){0};
        }
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_res_file_interface = {
    .init_fn = archi_context_res_file_init,
    .final_fn = archi_context_res_file_final,
    .get_fn = archi_context_res_file_get,
    .act_fn = archi_context_res_file_act,
};

