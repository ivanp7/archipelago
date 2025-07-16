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

#include "archi/builtin/res_file/file.var.h"
#include "archi/res/file/api.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

struct archi_context_res_file_data {
    archi_pointer_t mapping;
    int fd;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_file_init)
{
    archi_file_open_params_t file_open_params = {0};
    archi_file_open_params_t file_open_params_fields = {0};

    bool param_params_set = false;
    bool param_pathname_set = false;
    bool param_size_set = false;
    bool param_create_set = false;
    bool param_exclusive_set = false;
    bool param_truncate_set = false;
    bool param_readable_set = false;
    bool param_writable_set = false;
    bool param_nonblock_set = false;
    bool param_flags_set = false;
    bool param_mode_set = false;

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

            file_open_params = *(archi_file_open_params_t*)params->value.ptr;
        }
        else if (strcmp("size", params->name) == 0)
        {
            if (param_size_set)
                continue;
            param_size_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.size = *(size_t*)params->value.ptr;
        }
        else if (strcmp("pathname", params->name) == 0)
        {
            if (param_pathname_set)
                continue;
            param_pathname_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.pathname = params->value.ptr;
        }
        else if (strcmp("create", params->name) == 0)
        {
            if (param_create_set)
                continue;
            param_create_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.create = *(char*)params->value.ptr;
        }
        else if (strcmp("exclusive", params->name) == 0)
        {
            if (param_exclusive_set)
                continue;
            param_exclusive_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.exclusive = *(char*)params->value.ptr;
        }
        else if (strcmp("truncate", params->name) == 0)
        {
            if (param_truncate_set)
                continue;
            param_truncate_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.truncate = *(char*)params->value.ptr;
        }
        else if (strcmp("readable", params->name) == 0)
        {
            if (param_readable_set)
                continue;
            param_readable_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.readable = *(char*)params->value.ptr;
        }
        else if (strcmp("writable", params->name) == 0)
        {
            if (param_writable_set)
                continue;
            param_writable_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.writable = *(char*)params->value.ptr;
        }
        else if (strcmp("nonblock", params->name) == 0)
        {
            if (param_nonblock_set)
                continue;
            param_nonblock_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.nonblock = *(char*)params->value.ptr;
        }
        else if (strcmp("flags", params->name) == 0)
        {
            if (param_flags_set)
                continue;
            param_flags_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.flags = *(int*)params->value.ptr;
        }
        else if (strcmp("mode", params->name) == 0)
        {
            if (param_mode_set)
                continue;
            param_mode_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            file_open_params_fields.mode = *(int*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (param_pathname_set)
        file_open_params.pathname = file_open_params_fields.pathname;

    if (param_size_set)
        file_open_params.size = file_open_params_fields.size;

    if (param_create_set)
        file_open_params.create = file_open_params_fields.create;

    if (param_exclusive_set)
        file_open_params.exclusive = file_open_params_fields.exclusive;

    if (param_truncate_set)
        file_open_params.truncate = file_open_params_fields.truncate;

    if (param_readable_set)
        file_open_params.readable = file_open_params_fields.readable;

    if (param_writable_set)
        file_open_params.writable = file_open_params_fields.writable;

    if (param_nonblock_set)
        file_open_params.nonblock = file_open_params_fields.nonblock;

    if (param_flags_set)
        file_open_params.flags = file_open_params_fields.flags;

    if (param_mode_set)
        file_open_params.mode = file_open_params_fields.mode;

    struct archi_context_res_file_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    int fd = archi_file_open(file_open_params);
    if (fd < 0)
    {
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    *context_data = (struct archi_context_res_file_data){
        .fd = fd,
    };

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_file_final)
{
    struct archi_context_res_file_data *context_data =
        (struct archi_context_res_file_data*)context;

    archi_file_unmap(context_data->mapping.ptr, context_data->mapping.element.num_of);
    archi_file_close(context_data->fd);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_res_file_get)
{
    struct archi_context_res_file_data *context_data =
        (struct archi_context_res_file_data*)context;

    if (strcmp("fd", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->fd,
            .ref_count = context_data->mapping.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->fd),
                .alignment = alignof(int),
            },
        };
    }
    else if (strcmp("map", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->mapping;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_context_res_file_act)
{
    struct archi_context_res_file_data *context_data =
        (struct archi_context_res_file_data*)context;

    if (strcmp("map", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (context_data->mapping.ptr != NULL)
            return ARCHI_STATUS_EMISUSE;

        archi_file_map_params_t file_map_params = {0};
        archi_file_map_params_t file_map_params_fields = {0};
        bool close_fd = false;

        bool param_params_set = false;
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
                if (param_params_set)
                    continue;
                param_params_set = true;

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

                file_map_params_fields.size = *(size_t*)params->value.ptr;
            }
            else if (strcmp("offset", params->name) == 0)
            {
                if (param_offset_set)
                    continue;
                param_offset_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params_fields.offset = *(size_t*)params->value.ptr;
            }
            else if (strcmp("has_header", params->name) == 0)
            {
                if (param_has_header_set)
                    continue;
                param_has_header_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params_fields.has_header = *(char*)params->value.ptr;
            }
            else if (strcmp("readable", params->name) == 0)
            {
                if (param_readable_set)
                    continue;
                param_readable_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params_fields.readable = *(char*)params->value.ptr;
            }
            else if (strcmp("writable", params->name) == 0)
            {
                if (param_writable_set)
                    continue;
                param_writable_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params_fields.writable = *(char*)params->value.ptr;
            }
            else if (strcmp("shared", params->name) == 0)
            {
                if (param_shared_set)
                    continue;
                param_shared_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params_fields.shared = *(char*)params->value.ptr;
            }
            else if (strcmp("flags", params->name) == 0)
            {
                if (param_flags_set)
                    continue;
                param_flags_set = true;

                if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                        (params->value.ptr == NULL))
                    return ARCHI_STATUS_EVALUE;

                file_map_params_fields.flags = *(int*)params->value.ptr;
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

        if (param_size_set)
            file_map_params.size = file_map_params_fields.size;

        if (param_offset_set)
            file_map_params.offset = file_map_params_fields.offset;

        if (param_has_header_set)
            file_map_params.has_header = file_map_params_fields.has_header;

        if (param_readable_set)
            file_map_params.readable = file_map_params_fields.readable;

        if (param_writable_set)
            file_map_params.writable = file_map_params_fields.writable;

        if (param_shared_set)
            file_map_params.shared = file_map_params_fields.shared;

        if (param_flags_set)
            file_map_params.flags = file_map_params_fields.flags;

        size_t mm_size = 0;
        void *mm = archi_file_map(context_data->fd, file_map_params, &mm_size);

        if (mm == NULL)
            return ARCHI_STATUS_ERESOURCE;

        archi_reference_count_t ref_count = context_data->mapping.ref_count;
        context_data->mapping = (archi_pointer_t){
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
            archi_file_close(context_data->fd);
            context_data->fd = -1;
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

