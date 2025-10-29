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

#include "archi/file/ctx/mapping.var.h"
#include "archi/file/api/map.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__file_mapping)
{
    // Parse parameters
    int fd = -1;
    bool fd_set = false;
    size_t stride = 1;
    bool stride_set = false;
    size_t alignment = 1;
    bool alignment_set = false;
    archi_file_map_params_t file_map_params = {0};
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "fd", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_file_descriptor_t)},
            {.name = "stride", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "alignment", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "params", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_file_map_params_t)},
            {.name = "size", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "offset", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "ptr_support", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, char)},
            {.name = "readable", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, char)},
            {.name = "writable", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, char)},
            {.name = "shared", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, char)},
            {.name = "flags", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, int)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        fd_set = parsed[index].value_set;
        if (fd_set)
            fd = *(archi_file_descriptor_t*)parsed[index].value.ptr;
        index++;
        stride_set = parsed[index].value_set;
        if (stride_set)
            stride = *(size_t*)parsed[index].value.ptr;
        index++;
        alignment_set = parsed[index].value_set;
        if (alignment_set)
            alignment = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            file_map_params = *(archi_file_map_params_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            file_map_params.size = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            file_map_params.offset = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            file_map_params.ptr_support = *(char*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            file_map_params.readable = *(char*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            file_map_params.writable = *(char*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            file_map_params.shared = *(char*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            file_map_params.flags = *(int*)parsed[index].value.ptr;
    }

    // Check validity of parameters
    if (!fd_set)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor is not set");
        return NULL;
    }
    else if (fd < 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file descriptor (%i) is invalid", fd);
        return NULL;
    }

    if (file_map_params.ptr_support)
    {
        if (stride_set)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "stride specified for mapping of file with pointer support");
            return NULL;
        }
        else if (alignment_set)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "alignment specified for mapping of file with pointer support");
            return NULL;
        }
    }
    else
    {
        if (stride == 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file element stride is 0");
            return NULL;
        }
        else if (file_map_params.size % stride != 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file mapping size (%zu) is not divisible by stride (%zu)",
                    file_map_params.size, stride);
            return NULL;
        }
        else if (alignment == 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file element alignment requirement is 0");
            return NULL;
        }
        else if ((alignment & (alignment - 1)) != 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file element alignment requirement (%#zx) is not a power of two",
                    alignment);
            return NULL;
        }
        else if (stride % alignment != 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file element stride (%zu) is not divisible by alignment requirement (%#zx)",
                    stride, alignment);
            return NULL;
        }
    }

    size_t page_size = archi_file_page_size();

    if (file_map_params.offset % page_size != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file mapping offset (%zu) is not divisible by page size (%zu)",
                file_map_params.offset, page_size);
        return NULL;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    size_t mm_size = 0;
    void *mm = archi_file_map(fd, file_map_params, &mm_size, ARCHI_ERROR_PARAMETER);
    if (mm == NULL)
    {
        free(context_data);
        return NULL;
    }

    if (mm_size % stride != 0)
    {
        archi_file_unmap(mm, mm_size);
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "file mapping size (%zu) is not divisible by stride (%zu)",
                mm_size, stride);
        return NULL;
    }

    archi_pointer_attr_t attr = archi_pointer_attr__transp_data(mm_size / stride, stride, alignment,
            ARCHI_ERROR_PARAMETER);
    if (attr == (archi_pointer_attr_t)-1)
    {
        archi_file_unmap(mm, mm_size);
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = mm,
        .attr = (file_map_params.writable ?
                ARCHI_POINTER_TYPE__DATA_WRITABLE : ARCHI_POINTER_TYPE__DATA_READONLY) | attr,
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__file_mapping)
{
    size_t length, stride;
    archi_pointer_attr_parse__transp_data(context->attr, &length, &stride, NULL, NULL);

    archi_file_unmap(context->ptr, length * stride);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__file_mapping)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("length", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t length;
        archi_pointer_attr_parse__transp_data(context->attr, &length, NULL, NULL, NULL);

        archi_rcpointer_t value = {
            .ptr = &length,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("stride", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t stride;
        archi_pointer_attr_parse__transp_data(context->attr, NULL, &stride, NULL, NULL);

        archi_rcpointer_t value = {
            .ptr = &stride,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("size", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t length, stride;
        archi_pointer_attr_parse__transp_data(context->attr, &length, &stride, NULL, NULL);

        size_t size = length * stride;

        archi_rcpointer_t value = {
            .ptr = &size,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("alignment", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t alignment;
        archi_pointer_attr_parse__transp_data(context->attr, NULL, NULL, &alignment, NULL);

        archi_rcpointer_t value = {
            .ptr = &alignment,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__file_mapping = {
    .init_fn = archi_context_init__file_mapping,
    .final_fn = archi_context_final__file_mapping,
    .eval_fn = archi_context_eval__file_mapping,
};

