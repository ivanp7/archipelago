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
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"
#include "archi_base/util/size.def.h"

#include <stdlib.h> // for malloc(), free()


static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__file_mapping)
{
    // Parse parameters
    int fd = -1;
    size_t stride = 1, alignment = 1;
    archi_file_map_params_t file_map_params = {0};
    bool fd_set = false, stride_set = false, alignment_set = false;
    {
        archi_plist_param_t parsed[] = {
            {.name = "fd",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_file_descriptor_t)}},
                .assign = {archi_plist_assign__value, &fd, sizeof(fd), &fd_set}},
            {.name = "stride",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &stride, sizeof(stride), &stride_set}},
            {.name = "alignment",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &alignment, sizeof(alignment), &alignment_set}},
            {.name = "params",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_file_map_params_t)}},
                .assign = {archi_plist_assign__value, &file_map_params, sizeof(file_map_params)}},
            {.name = "size",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &file_map_params.size, sizeof(file_map_params.size)}},
            {.name = "offset",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &file_map_params.offset, sizeof(file_map_params.offset)}},
            {.name = "ptr_support",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_map_params.ptr_support, sizeof(file_map_params.ptr_support)}},
            {.name = "readable",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_map_params.readable, sizeof(file_map_params.readable)}},
            {.name = "writable",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_map_params.writable, sizeof(file_map_params.writable)}},
            {.name = "shared",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &file_map_params.shared, sizeof(file_map_params.shared)}},
            {.name = "flags",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &file_map_params.flags, sizeof(file_map_params.flags)}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Check validness of parameters
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
        else if (!ARCHI_ALIGNMENT_VALID(alignment))
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
    void *mm = archi_file_map(fd, file_map_params, &mm_size, ARCHI_ERROR_PARAM);
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

    archi_pointer_attr_t attr = archi_pointer_attr__pdata(mm_size / stride, stride, alignment,
            ARCHI_ERROR_PARAM);
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
    archi_pointer_attr_unpk__pdata(context->attr, &length, &stride, NULL, NULL);

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
        archi_pointer_attr_unpk__pdata(context->attr, &length, NULL, NULL, NULL);

        archi_rcpointer_t value = {
            .ptr = &length,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
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
        archi_pointer_attr_unpk__pdata(context->attr, NULL, &stride, NULL, NULL);

        archi_rcpointer_t value = {
            .ptr = &stride,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
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
        archi_pointer_attr_unpk__pdata(context->attr, &length, &stride, NULL, NULL);

        size_t size = length * stride;

        archi_rcpointer_t value = {
            .ptr = &size,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
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
        archi_pointer_attr_unpk__pdata(context->attr, NULL, NULL, &alignment, NULL);

        archi_rcpointer_t value = {
            .ptr = &alignment,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
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

