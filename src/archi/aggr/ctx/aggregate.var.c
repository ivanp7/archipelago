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
 * @brief Context interface for aggregate objects.
 */

#include "archi/aggr/ctx/aggregate.var.h"
#include "archi/aggr/api/interface.fun.h"
#include "archi/aggr/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__aggregate)
{
    // Parse parameters
    archi_rcpointer_t interface = {0}, metadata = {0};
    size_t fam_length = 0;
    {
        archi_plist_param_t parsed[] = {
            {.name = "interface",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE)}},
                .assign = {archi_plist_assign__rcpointer, &interface, sizeof(interface), NULL}},
            {.name = "metadata",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                .assign = {archi_plist_assign__rcpointer, &metadata, sizeof(metadata), NULL}},
            {.name = "fam_length",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &fam_length, sizeof(fam_length), NULL}},
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

    archi_aggr_t aggregate = archi_aggr_allocate(interface, metadata, fam_length,
            ARCHI_ERROR_PARAM);
    if (aggregate == NULL)
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = aggregate,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__AGGR),
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__aggregate)
{
    archi_aggr_free(context->ptr);
    free(context);
}

static
size_t
archi_context_slot__member_depth(
        const char *slot_name,
        size_t prefix_length)
{
    // Count member depth (number of components between dots '.')
    size_t member_depth = 1;

    slot_name += prefix_length;
    while ((slot_name = strchr(slot_name, '.')) != NULL)
    {
        slot_name++; // go past '.'
        member_depth++;
    }

    return member_depth;
}

static
archi_aggr_member_spec_t*
archi_context_slot__member_spec(
        archi_context_slot_t slot,
        size_t member_depth,
        size_t prefix_length,
        ARCHI_ERROR_PARAM_DECL)
{
    // Check correctness of slot indices
    if (slot.num_indices > member_depth)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices is greater than member depth (%zu)",
                member_depth);
        return NULL;
    }

    for (size_t i = 0; i < slot.num_indices; i++)
    {
        if (slot.index[i] < 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "negative index was provided");
            return NULL;
        }
        else if ((size_t)slot.index[i] > SIZE_MAX)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index is greater than SIZE_MAX");
            return NULL;
        }
    }

    // Prepare array of member specifiers
    archi_aggr_member_spec_t *member_spec = malloc(sizeof(*member_spec) * member_depth);
    if (member_spec == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of aggregate member specifiers");
        return NULL;
    }

    char *current_member_name = archi_string_copy(slot.name + prefix_length);
    if (current_member_name == NULL)
    {
        free(member_spec);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate slot name copy");
        return NULL;
    }

    for (size_t i = 0; i < member_depth; i++)
    {
        member_spec[i].name = current_member_name;
        member_spec[i].index = (i < slot.num_indices) ? slot.index[i] : 0;

        // Proceed to the next component, splitting the string
        current_member_name = strchr(current_member_name, '.');
        if (current_member_name != NULL)
        {
            *current_member_name = '\0'; // split the string at the dot
            current_member_name++; // point to the next component beginning
        }
    }

    return member_spec;
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__aggregate)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("interface", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(archi_aggr_interface(context->ptr));
    }
    else if (ARCHI_STRING_COMPARE("metadata", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(archi_aggr_metadata(context->ptr));
    }
    else if (ARCHI_STRING_COMPARE("layout", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_layout_struct_t layout = archi_aggr_layout(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &layout,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, archi_layout_struct_t),
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

        size_t size = archi_aggr_layout(context->ptr).base.size;

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

        size_t alignment = archi_aggr_layout(context->ptr).base.alignment;

        archi_rcpointer_t value = {
            .ptr = &alignment,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("fam_stride", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t fam_stride = archi_aggr_layout(context->ptr).fam_stride;

        archi_rcpointer_t value = {
            .ptr = &fam_stride,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("fam_length", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t fam_length = archi_aggr_fam_length(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &fam_length,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("full_size", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t full_size = archi_aggr_full_size(context->ptr);

        archi_rcpointer_t value = {
            .ptr = &full_size,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("object", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(archi_aggr_object(context->ptr));
    }
#define PREFIX_LENGTH 7 // strlen("member.")
    else if (strncmp("member.", slot.name, PREFIX_LENGTH) == 0)
    {
        // Prepare array of member specifiers
        size_t member_depth = archi_context_slot__member_depth(slot.name, PREFIX_LENGTH);

        archi_aggr_member_spec_t *member_spec =
            archi_context_slot__member_spec(slot, member_depth, PREFIX_LENGTH, ARCHI_ERROR_PARAM);
        if (member_spec == NULL)
            return;

        ARCHI_ERROR_VAR(error);

        // Get the member
        archi_rcpointer_t value = archi_aggr_get(context->ptr, member_spec, member_depth, false, &error);
        ARCHI_ERROR_ASSIGN(error);

        free((char*)member_spec[0].name);
        free(member_spec);

        if (error.code != 0)
            ARCHI_CONTEXT_YIELD(value);
    }
#undef PREFIX_LENGTH
#define PREFIX_LENGTH 4 // strlen("ref.")
    else if (strncmp("ref.", slot.name, PREFIX_LENGTH) == 0)
    {
        // Prepare array of member specifiers
        size_t member_depth = archi_context_slot__member_depth(slot.name, PREFIX_LENGTH);

        archi_aggr_member_spec_t *member_spec =
            archi_context_slot__member_spec(slot, member_depth, PREFIX_LENGTH, ARCHI_ERROR_PARAM);
        if (member_spec == NULL)
            return;

        ARCHI_ERROR_VAR(error);

        // Get the member
        archi_rcpointer_t value = archi_aggr_get(context->ptr, member_spec, member_depth, true, &error);
        ARCHI_ERROR_ASSIGN(error);

        free((char*)member_spec[0].name);
        free(member_spec);

        if (error.code != 0)
            ARCHI_CONTEXT_YIELD(value);
    }
#undef PREFIX_LENGTH
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__aggregate)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

#define PREFIX_LENGTH 7 // strlen("member.")
    if (strncmp("member.", slot.name, PREFIX_LENGTH) == 0)
    {
        // Prepare array of member specifiers
        size_t member_depth = archi_context_slot__member_depth(slot.name, PREFIX_LENGTH);

        archi_aggr_member_spec_t *member_spec =
            archi_context_slot__member_spec(slot, member_depth, PREFIX_LENGTH, ARCHI_ERROR_PARAM);
        if (member_spec == NULL)
            return;

        // Set the member
        archi_aggr_set(context->ptr, member_spec, member_depth, value, ARCHI_ERROR_PARAM);

        free((char*)member_spec[0].name);
        free(member_spec);
    }
#undef PREFIX_LENGTH
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }
}

const archi_context_interface_t
archi_context_interface__aggregate = {
    .init_fn = archi_context_init__aggregate,
    .final_fn = archi_context_final__aggregate,
    .eval_fn = archi_context_eval__aggregate,
    .set_fn = archi_context_set__aggregate,
};

