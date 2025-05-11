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
#include <stdalign.h>

struct archi_context_res_file {
    int fd;   ///< File descriptor.

    void *mm; ///< Mapped memory.
    size_t mm_size; ///< Mapped memory size.
};

/*****************************************************************************/

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

            file_open_params.readable = *(bool*)params->value.ptr;
        }
        else if (strcmp("writable", params->name) == 0)
        {
            if (param_writable_set)
                continue;
            param_writable_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params.writable = *(bool*)params->value.ptr;
        }
        else if (strcmp("nonblock", params->name) == 0)
        {
            if (param_nonblock_set)
                continue;
            param_nonblock_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params.nonblock = *(bool*)params->value.ptr;
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

    struct archi_context_res_file *file = malloc(sizeof(*file));
    if (file == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    *file = (struct archi_context_res_file){.fd = archi_file_open(file_open_params)};
    if (file->fd < 0)
    {
        free(file);
        return ARCHI_STATUS_ERESOURCE;
    }

    context->public_value = (archi_pointer_t){
        .ptr = file,
        .element = {
            .num_of = 1,
        },
    };

    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_file_final)
{
    struct archi_context_res_file *file = context.public_value.ptr;

    archi_file_unmap(file->mm, file->mm_size);
    archi_file_close(file->fd);
    free(file);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_file_get)
{
    struct archi_context_res_file *file = context.public_value.ptr;

    if (strcmp("fd", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &file->fd,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(file->fd),
                .alignment = alignof(int),
            },
        };
    }
    else if (strcmp("map", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = file->mm,
            .ref_count = context.public_value.ref_count,
            .element = {
                .num_of = file->mm_size,
                .size = 1,
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_res_file_act)
{
    struct archi_context_res_file *file = context.public_value.ptr;

    if (strcmp("map", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        if ((file->fd < 0) || (file->mm != NULL))
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
            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            if (strcmp("params", params->name) == 0)
            {
                if (param_struct_set)
                    continue;
                param_struct_set = true;

                file_map_params = *(archi_file_map_params_t*)params->value.ptr;
            }
            else if (strcmp("size", params->name) == 0)
            {
                if (param_size_set)
                    continue;
                param_size_set = true;

                file_map_params.size = *(size_t*)params->value.ptr;
            }
            else if (strcmp("offset", params->name) == 0)
            {
                if (param_offset_set)
                    continue;
                param_offset_set = true;

                file_map_params.offset = *(size_t*)params->value.ptr;
            }
            else if (strcmp("has_header", params->name) == 0)
            {
                if (param_has_header_set)
                    continue;
                param_has_header_set = true;

                file_map_params.has_header = *(bool*)params->value.ptr;
            }
            else if (strcmp("readable", params->name) == 0)
            {
                if (param_readable_set)
                    continue;
                param_readable_set = true;

                file_map_params.readable = *(bool*)params->value.ptr;
            }
            else if (strcmp("writable", params->name) == 0)
            {
                if (param_writable_set)
                    continue;
                param_writable_set = true;

                file_map_params.writable = *(bool*)params->value.ptr;
            }
            else if (strcmp("shared", params->name) == 0)
            {
                if (param_shared_set)
                    continue;
                param_shared_set = true;

                file_map_params.shared = *(bool*)params->value.ptr;
            }
            else if (strcmp("flags", params->name) == 0)
            {
                if (param_flags_set)
                    continue;
                param_flags_set = true;

                file_map_params.flags = *(int*)params->value.ptr;
            }
            else if (strcmp("close_fd", params->name) == 0)
            {
                if (param_close_fd_set)
                    continue;
                param_close_fd_set = true;

                close_fd = *(bool*)params->value.ptr;
            }
            else
                return ARCHI_STATUS_EKEY;
        }

        file->mm = archi_file_map(file->fd, file_map_params, &file->mm_size);
        if (file->mm == NULL)
            return ARCHI_STATUS_ERESOURCE;

        if (close_fd)
        {
            archi_file_close(file->fd);
            file->fd = -1;
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

