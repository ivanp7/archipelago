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
 * @brief Context interface for shared libraries.
 */

#include "archi/library/ctx/library.var.h"
#include "archi/library/api/library.fun.h"
#include "archi/library/api/global.fun.h"
#include "archi/library/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/global.var.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__library)
{
    // Parse parameters
    const char *pathname = NULL;
    archi_library_load_params_t library_load_params = {0};
    {
        archi_plist_param_t parsed[] = {
            {.name = "pathname",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(0, char)}},
                .assign = {archi_plist_assign__dptr_n, &pathname, sizeof(pathname), NULL}},
            {.name = "params",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_library_load_params_t)}},
                .assign = {archi_plist_assign__value, &library_load_params, sizeof(library_load_params), NULL}},
            {.name = "lazy",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &library_load_params.lazy, sizeof(library_load_params.lazy), NULL}},
            {.name = "global",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &library_load_params.global, sizeof(library_load_params.global), NULL}},
            {.name = "flags",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, int)}},
                .assign = {archi_plist_assign__value, &library_load_params.flags, sizeof(library_load_params.flags), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_library_handle_t library = archi_library_load(pathname, library_load_params, ARCHI_ERROR_PARAM);
    if (library == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = library,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__LIBRARY_HANDLE),
    };

    // Initialize static context pointers in the loaded library
    for (size_t i = 0; archi_global_init_specs[i].get_fn != NULL; i++)
        archi_library_initialize_global(library, archi_global_init_specs[i]);

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__library)
{
    archi_library_unload(context->ptr);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__library)
{
    if (slot.num_indices != 0)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
        return;
    }

    // Extract symbol name and type
    bool function;
    const char *symbol_name;
    {
        if (strncmp("data.", slot.name, 5) == 0)
        {
            function = false;
            symbol_name = &slot.name[5];
        }
        else if (strncmp("function.", slot.name, 9) == 0)
        {
            function = true;
            symbol_name = &slot.name[9];
        }
        else
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
            return;
        }
    }

    // Compute symbol attributes
    archi_pointer_attr_t symbol_attr;

    archi_pointer_attr_t tag = 0;
    size_t length = 1, stride = 1, alignment = 1;
    bool tag_set = false, length_stride_alignment_set = false;

    if (call)
    {
        // Parse parameters
        if (!function)
        {
            archi_plist_param_t parsed[] = {
                {.name = "tag",
                    .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_pointer_attr_t)}},
                    .assign = {archi_plist_assign__value, &tag, sizeof(tag), &tag_set}},
                {.name = "length",
                    .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                    .assign = {archi_plist_assign__value, &length, sizeof(length), &length_stride_alignment_set}},
                {.name = "stride",
                    .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                    .assign = {archi_plist_assign__value, &stride, sizeof(stride), &length_stride_alignment_set}},
                {.name = "alignment",
                    .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                    .assign = {archi_plist_assign__value, &alignment, sizeof(alignment), &length_stride_alignment_set}},
                {0},
            };

            if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
                return;

            if (tag_set && length_stride_alignment_set)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data symbol can't have tag specified simultaneously with length/stride/alignment");
                return;
            }
        }
        else
        {
            archi_plist_param_t parsed[] = {
                {.name = "tag",
                    .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_pointer_attr_t)}},
                    .assign = {archi_plist_assign__value, &tag, sizeof(tag), &tag_set}},
                {0},
            };

            if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
                return;
        }
    }

    if (!function)
    {
        if (length_stride_alignment_set) // primitive data
        {
            ARCHI_ERROR_VAR(error);

            symbol_attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                archi_pointer_attr__pdata(length, stride, alignment, &error);
            if (symbol_attr == (archi_pointer_attr_t)-1)
            {
                ARCHI_ERROR_SET(error.code, "primitive data pointer attributes are invalid: %s", error.message);
                return;
            }
        }
        else // complex data
        {
            symbol_attr = ARCHI_POINTER_TYPE__DATA_READONLY | archi_pointer_attr__cdata(tag);
            if (symbol_attr == (archi_pointer_attr_t)-1)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "complex data type tag (%llu) exceeds maximum value",
                        (unsigned long long)tag);
                return;
            }
        }
    }
    else
    {
        symbol_attr = archi_pointer_attr__func(tag);
        if (symbol_attr == 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "function type tag (%llu) exceeds maximum value",
                    (unsigned long long)tag);
            return;
        }
    }

    // Extract the symbol
    archi_rcpointer_t symbol = {
        .ptr = archi_library_get_symbol(context->ptr, symbol_name),
        .attr = symbol_attr,
        .ref_count = ARCHI_CONTEXT_REF_COUNT,
    };

    if (symbol.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "symbol '%s' not found", symbol_name);
        return;
    }

    ARCHI_ERROR_VAR(error);
    if (!archi_pointer_valid(symbol.p, &error))
    {
        ARCHI_ERROR_SET(error.code, "symbol '%s' pointer is invalid: %s", symbol_name, error.message);
        return;
    }

    ARCHI_CONTEXT_YIELD(symbol);
}

const archi_context_interface_t
archi_context_interface__library = {
    .init_fn = archi_context_init__library,
    .final_fn = archi_context_final__library,
    .eval_fn = archi_context_eval__library,
};

