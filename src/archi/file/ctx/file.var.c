/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
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
 * @brief Context interface for files.
 */

#include "archi/file/ctx/file.var.h"
#include "archi/file/api/file.fun.h"
#include "archi/file/api/stream.fun.h"
#include "archi/file/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


struct archi_context_data__file {
    archi_rcpointer_t stream;
    archi_file_descriptor_t fd;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__file)
{
    // Parse parameters
    const char *pathname = NULL;
    archi_file_descriptor_t fd = -1;
    archi_file_open_params_t file_open_params = {0};
    bool stream = false;
    bool pathname_set = false, fd_set = false;
    {
        archi_plist_param_t parsed[] = {
            {.name = "pathname",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__dptr, &pathname, sizeof(pathname), &pathname_set}},
            {.name = "fd",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_file_descriptor_t)}},
                .assign = {archi_plist_assign__value, &fd, sizeof(fd), &fd_set}},
            {.name = "params",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_file_open_params_t)}},
                .assign = {archi_plist_assign__value, &file_open_params, sizeof(file_open_params)}},
            {.name = "size",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &file_open_params.size, sizeof(file_open_params.size)}},
            {.name = "readable",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_open_params.readable, sizeof(file_open_params.readable)}},
            {.name = "writable",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_open_params.writable, sizeof(file_open_params.writable)}},
            {.name = "create",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_open_params.create, sizeof(file_open_params.create)}},
            {.name = "exclusive",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_open_params.exclusive, sizeof(file_open_params.exclusive)}},
            {.name = "truncate",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_open_params.truncate, sizeof(file_open_params.truncate)}},
            {.name = "append",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_open_params.append, sizeof(file_open_params.append)}},
            {.name = "flags",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &file_open_params.flags, sizeof(file_open_params.flags)}},
            {.name = "mode",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &file_open_params.mode, sizeof(file_open_params.mode)}},
            {.name = "stream",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &stream, sizeof(stream)}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Check validness of parameters
    if (!pathname_set && !fd_set)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "neither pathname nor file descriptor aren't provided");
        return NULL;
    }
    else if (pathname_set && fd_set)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pathname and file descriptor are provided simultaneously");
        return NULL;
    }

    // Construct the context
    struct archi_context_data__file *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__file){
        .stream = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__FILE_STREAM),
        },
    };

    if (pathname_set)
        context_data->fd = archi_file_open(pathname, file_open_params, ARCHI_ERROR_PARAM);
    else if (fd_set)
        context_data->fd = archi_file_duplicate(fd, ARCHI_ERROR_PARAM);

    if (context_data->fd < 0)
    {
        free(context_data);
        return NULL;
    }

    if (stream)
    {
        context_data->stream.ptr = archi_file_stream_open(context_data->fd, file_open_params, ARCHI_ERROR_PARAM);
        if (context_data->stream.ptr == NULL)
        {
            archi_file_close(context_data->fd, NULL);
            free(context_data);
            return NULL;
        }
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__file)
{
    struct archi_context_data__file *context_data =
        (struct archi_context_data__file*)context;

    if (context_data->stream.ptr != NULL)
        archi_file_stream_close(context_data->stream.ptr, NULL);
    else
        archi_file_close(context_data->fd, NULL);

    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__file)
{
    struct archi_context_data__file *context_data =
        (struct archi_context_data__file*)context;

    if (!call)
    {
        if (ARCHI_STRING_COMPARE("fd", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            archi_file_descriptor_t fd = context_data->fd;

            archi_rcpointer_t value = {
                .ptr = &fd,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, archi_file_descriptor_t),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("offset", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            long long offset = archi_file_seek(context_data->fd, 0, SEEK_CUR, ARCHI_ERROR_PARAM);
            if (offset < 0)
                return;

            archi_rcpointer_t value = {
                .ptr = &offset,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, long long),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
    }
    else
    {
        if (ARCHI_STRING_COMPARE("read", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            // Parse parameters
            archi_rcpointer_t destination = {0};
            size_t dest_offset = 0, length = 0;
            bool destination_set = false, length_set = false;
            {
                archi_plist_param_t parsed[] = {
                    {.name = "dest",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                        .assign = {archi_plist_assign__rcpointer, &destination, sizeof(destination), &destination_set}},
                    {.name = "dest_offset",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &dest_offset, sizeof(dest_offset)}},
                    {.name = "length",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &length, sizeof(length), &length_set}},
                    {0},
                };

                if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
                    return;
            }

            // Check validity of the parameters
            if (!destination_set)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination pointer is not set");
                return;
            }

            size_t dest_length, dest_stride;
            {
                ARCHI_ERROR_VAR(error);

                if (!archi_pointer_attr_unpk__pdata(destination.attr, &dest_length, &dest_stride, NULL, &error))
                {
                    ARCHI_ERROR_SET(error.code, "destination pointer does not refer to primitive data: %s", error.message);
                    return;
                }
            }

            if (dest_offset > dest_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination offset (%zu) exceeds destination length (%zu)",
                        dest_offset, dest_length);
                return;
            }

            if (!length_set)
                length = dest_length - dest_offset;

            if ((length != 0) && (destination.ptr == NULL))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination pointer is NULL for non-zero read length");
                return;
            }

            if (dest_offset + length > dest_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "read area (offset = %zu, length = %zu) is out of destination bounds (length = %zu)",
                        dest_offset, length, dest_length);
                return;
            }

            // Read the data
            ARCHI_ERROR_VAR(error);

            size_t num_read = archi_file_read(context_data->fd, destination.p, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;

            if (num_read != dest_length * dest_stride)
            {
                ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't read the whole area (read = %zu, expected = %zu)",
                        num_read, dest_length * dest_stride);
                return;
            }
        }
        else if (ARCHI_STRING_COMPARE("write", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            // Parse parameters
            archi_rcpointer_t source = {0};
            size_t src_offset = 0, length = 0;
            bool source_set = false, length_set = false;
            {
                archi_plist_param_t parsed[] = {
                    {.name = "src",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                        .assign = {archi_plist_assign__rcpointer, &source, sizeof(source), &source_set}},
                    {.name = "src_offset",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &src_offset, sizeof(src_offset)}},
                    {.name = "length",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &length, sizeof(length), &length_set}},
                    {0},
                };

                if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
                    return;
            }

            // Check validity of the parameters
            if (!source_set)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source pointer is not set");
                return;
            }

            size_t src_length, src_stride;
            {
                ARCHI_ERROR_VAR(error);

                if (!archi_pointer_attr_unpk__pdata(source.attr, &src_length, &src_stride, NULL, &error))
                {
                    ARCHI_ERROR_SET(error.code, "source pointer does not refer to primitive data: %s", error.message);
                    return;
                }
            }

            if (src_offset > src_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source offset (%zu) exceeds source length (%zu)",
                        src_offset, src_length);
                return;
            }

            if (!length_set)
                length = src_length - src_offset;

            if ((length != 0) && (source.ptr == NULL))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source pointer is NULL for non-zero write length");
                return;
            }

            if (src_offset + length > src_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "written area (offset = %zu, length = %zu) is out of source bounds (length = %zu)",
                        src_offset, length, src_length);
                return;
            }

            // Write the data
            ARCHI_ERROR_VAR(error);

            size_t num_written = archi_file_write(context_data->fd, source.p, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;

            if (num_written != src_length * src_stride)
            {
                ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't write the whole are (written = %zu, expected = %zu)",
                        num_written, src_length * src_stride);
                return;
            }
        }
        else if (ARCHI_STRING_COMPARE("sync", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }
            else if (params != NULL)
            {
                ARCHI_ERROR_SET(ARCHI__EKEY, "call doesn't accept parameters");
                return;
            }

            // Synchronize the file state with storage
            ARCHI_ERROR_VAR(error);

            archi_file_sync(context_data->fd, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }
        else
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
            return;
        }

        ARCHI_ERROR_RESET();
    }
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__file)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

    struct archi_context_data__file *context_data =
        (struct archi_context_data__file*)context;

    if (ARCHI_STRING_COMPARE("offset", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__PDATA(1, long long)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a long long");
            return;
        }

        long long offset = *(long long*)value.ptr;

        offset = archi_file_seek(context_data->fd, offset, SEEK_SET, ARCHI_ERROR_PARAM);
        if (offset < 0)
            return;
    }
    else if (ARCHI_STRING_COMPARE("offset.end", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__PDATA(1, long long)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a long long");
            return;
        }

        long long offset = *(long long*)value.ptr;

        offset = archi_file_seek(context_data->fd, offset, SEEK_END, ARCHI_ERROR_PARAM);
        if (offset < 0)
            return;
    }
    else if (ARCHI_STRING_COMPARE("offset.shift", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__PDATA(1, long long)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a long long");
            return;
        }

        long long offset = *(long long*)value.ptr;

        offset = archi_file_seek(context_data->fd, offset, SEEK_CUR, ARCHI_ERROR_PARAM);
        if (offset < 0)
            return;
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__file = {
    .init_fn = archi_context_init__file,
    .final_fn = archi_context_final__file,
    .eval_fn = archi_context_eval__file,
    .set_fn = archi_context_set__file,
};

