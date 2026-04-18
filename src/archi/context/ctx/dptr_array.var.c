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
 * @brief Context interface for arrays of data pointers.
 */

#include "archi/context/ctx/dptr_array.var.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/dptr_array.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/tag.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


struct archi_context_data__dptr_array {
    archi_rcpointer_t array;

    // References
    archi_rcpointer_t *ref_element;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__dptr_array)
{
    // Parse parameters
    size_t length = 0;
    {
        archi_plist_param_t parsed[] = {
            {.name = "length",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &length, sizeof(length)}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Check validness of parameters
    if (length >= (ARCHI_POINTER_DATA_SIZE_MAX + 1) / sizeof(archi_rcpointer_t))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array length (%zu) is too big", length);
        return NULL;
    }

    // Construct the context
    struct archi_context_data__dptr_array *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__dptr_array){
        .array = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DPTR_ARRAY),
        },
    };

    context_data->array.ptr = archi_dptr_array_alloc(length);
    if (context_data->array.ptr == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array [%zu] of data pointers", length);
        return NULL;
    }

    if (length > 0)
    {
        context_data->ref_element = malloc(sizeof(archi_rcpointer_t) * length);
        if (context_data->ref_element == NULL)
        {
            free(context_data->array.ptr);
            free(context_data);

            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array [%zu] of pointer wrappers", length);
            return NULL;
        }

        for (size_t i = 0; i < length; i++)
            context_data->ref_element[i] = (archi_rcpointer_t){0};
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__dptr_array)
{
    struct archi_context_data__dptr_array *context_data =
        (struct archi_context_data__dptr_array*)context;

    archi_dptr_array_t array = context_data->array.ptr;

    for (size_t i = 0; i < array->length; i++)
        archi_rcpointer_disown(context_data->ref_element[i]);

    free(context_data->ref_element);
    free(context_data->array.ptr);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__dptr_array)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__dptr_array *context_data =
        (struct archi_context_data__dptr_array*)context;

    archi_dptr_array_t array = context_data->array.ptr;

    if (ARCHI_STRING_COMPARE("", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }

        archi_context_slot_index_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= array->length))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %zu)", index, array->length);
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->ref_element[index]);
    }
    else if (ARCHI_STRING_COMPARE("ptr", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }

        archi_context_slot_index_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= array->length))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %zu)", index, array->length);
            return;
        }

        archi_rcpointer_t value = {
            .ptr = &array->ptr[index],
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                ARCHI_POINTER_ATTR__PDATA(1, archi_data_t),
            .ref_count = ARCHI_CONTEXT_REF_COUNT,
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("ptrs", ==, slot.name))
    {
        if (slot.num_indices > 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0 or 1");
            return;
        }

        archi_context_slot_index_t index = 0;
        if (slot.num_indices > 0)
        {
            index = slot.index[0];
            if ((index < 0) || ((size_t)index >= array->length))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %zu)", index, array->length);
                return;
            }
        }

        archi_rcpointer_t value = {
            .ptr = &array->ptr[index],
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                ARCHI_POINTER_ATTR__PDATA(array->length - index, archi_data_t),
            .ref_count = ARCHI_CONTEXT_REF_COUNT,
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("length", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t length = array->length;

        archi_rcpointer_t value = {
            .ptr = &length,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__dptr_array)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

    struct archi_context_data__dptr_array *context_data =
        (struct archi_context_data__dptr_array*)context;

    archi_dptr_array_t array = context_data->array.ptr;

    if (ARCHI_STRING_COMPARE("", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "function pointers aren't supported");
            return;
        }

        archi_context_slot_index_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= array->length))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %zu)", index, array->length);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_element[index], ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        array->ptr[index] = value.ptr;
        context_data->ref_element[index] = value;
    }
    else if (ARCHI_STRING_COMPARE("length", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__PDATA(1, size_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a size_t");
            return;
        }

        size_t new_length = *(size_t*)value.ptr;
        if (new_length >= (ARCHI_POINTER_DATA_SIZE_MAX + 1) / sizeof(archi_rcpointer_t))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array length (%zu) is too big", new_length);
            return;
        }

        // Allocate new arrays
        archi_dptr_array_t new_array = archi_dptr_array_alloc(new_length);
        if (new_array == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array [%zu] of pointers", new_length);
            return;
        }

        archi_rcpointer_t *new_ref_element = NULL;
        if (new_length > 0)
        {
            new_ref_element = malloc(sizeof(archi_rcpointer_t) * new_length);
            if (new_ref_element == NULL)
            {
                free(new_array);

                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array [%zu] of pointer wrappers", new_length);
                return;
            }
        }

        // Copy contents of the arrays
        if (new_length <= array->length)
        {
            for (size_t i = 0; i < new_length; i++)
            {
                new_array->ptr[i] = array->ptr[i];
                new_ref_element[i] = context_data->ref_element[i];
            }

            for (size_t i = new_length; i < array->length; i++)
                archi_rcpointer_disown(context_data->ref_element[i]);
        }
        else
        {
            for (size_t i = 0; i < array->length; i++)
            {
                new_array->ptr[i] = array->ptr[i];
                new_ref_element[i] = context_data->ref_element[i];
            }

            for (size_t i = array->length; i < new_length; i++)
                new_ref_element[i] = (archi_rcpointer_t){0};
        }

        // Deallocate old arrays and replace the pointers
        free(context_data->array.ptr);
        free(context_data->ref_element);

        context_data->array.ptr = new_array;
        context_data->ref_element = new_ref_element;
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__dptr_array = {
    .init_fn = archi_context_init__dptr_array,
    .final_fn = archi_context_final__dptr_array,
    .eval_fn = archi_context_eval__dptr_array,
    .set_fn = archi_context_set__dptr_array,
};

