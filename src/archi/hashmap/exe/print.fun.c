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
 * @brief DEG operation function for printing hashmap key-value pairs to the log.
 */

#include "archi/hashmap/exe/print.fun.h"
#include "archi/hashmap/api/hashmap.fun.h"
#include "archi/hashmap/api/tag.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"

#include <stdio.h>


#define INDENTATION     4

static
ARCHI_HASHMAP_TRAV_KV_FUNC(archi_dexgraph_op__hashmap_print_traverse)
{
    (void) data;

    printf("%*s[%zu] %s = ", INDENTATION, "", index, key);

    if (!archi_pointer_valid(value.p, NULL))
        printf("(INVALID) ");

    if (value.ptr != NULL)
        printf("%p", value.ptr);
    else
        printf("NULL");

    printf(" <");

    size_t length, stride, alignment;
    archi_pointer_attr_t tag;

    if (archi_pointer_attr_unpk__pdata(value.attr, &length, &stride, &alignment, NULL))
    {
        const char *mem_type;

        if (ARCHI_POINTER_TO_READONLY_DATA(value.attr))
            mem_type = "read-only";
        else if (ARCHI_POINTER_TO_WRITABLE_DATA(value.attr))
            mem_type = "writable";
        else
            mem_type = "on stack";

        printf("data, %s, length = %zu, stride = %zu, alignment = %#zx",
                mem_type, length, stride, alignment);
    }
    else if (archi_pointer_attr_unpk__cdata(value.attr, &tag, NULL))
    {
        const char *mem_type;

        if (ARCHI_POINTER_TO_READONLY_DATA(value.attr))
            mem_type = "read-only";
        else if (ARCHI_POINTER_TO_WRITABLE_DATA(value.attr))
            mem_type = "writable";
        else
            mem_type = "on stack";

        printf("data, %s, tag = %llu",
                mem_type, (unsigned long long)tag);
    }
    else if (archi_pointer_attr_unpk__func(value.attr, &tag, NULL))
    {
        printf("function, tag = %llu",
                (unsigned long long)tag);
    }

    printf(">");

    if (value.ref_count != NULL)
        printf(" (refcount %p)", (void*)value.ref_count);

    printf("\n");

    return (archi_hashmap_trav_action_t){0};
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__hashmap_print)
{
    if (data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "hashmap is NULL");
        return;
    }

    archi_hashmap_traverse(data, true, archi_dexgraph_op__hashmap_print_traverse, NULL,
            ARCHI_ERROR_PARAM);
}

