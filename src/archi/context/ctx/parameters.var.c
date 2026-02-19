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
 * @brief Context interface for parameter lists.
 */

#include "archi/context/ctx/parameters.var.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/kvlist.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/ref_count.fun.h"
#include "archi_base/tag.def.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <stdbool.h>


static
archi_krcvlist_t*
archi_context_parameters_copy(
        const archi_krcvlist_t *params,
        archi_krcvlist_t *base,
        ARCHI_ERROR_PARAM_DECL)
{
    if (params == NULL)
        return NULL;

    // Copy the parameter list
    archi_krcvlist_t *head, *tail;
    head = archi_krcvlist_copy(params, &tail);
    if (head == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate parameter list copy");
        return NULL;
    }

    // Increment reference counters of values, copy stack memory
    for (archi_krcvlist_t *node = head; node != NULL; node = (archi_krcvlist_t*)node->next)
    {
        node->value = archi_rcpointer_own(node->value, ARCHI_ERROR_PARAM);
        if (!node->value.attr) // failed to own
        {
            // Undo the work
            for (const archi_krcvlist_t *node2 = head; node2 != node; node2 = node2->next)
                archi_rcpointer_disown(node2->value);

            archi_krcvlist_free(head, false);
            return NULL;
        }
    }

    if (tail != NULL)
        tail->next = base;

    return head;
}

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__parameters)
{
    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_krcvlist_t *copy = archi_context_parameters_copy(params, NULL,
            ARCHI_ERROR_PARAM);
    if ((copy == NULL) && (params != NULL))
    {
        free(context_data);
        return NULL;
    }

    *context_data = (archi_rcpointer_t){
        .ptr = copy,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__KRCVLIST),
    };

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__parameters)
{
    archi_krcvlist_free(context->ptr, true);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__parameters)
{
    if (!call)
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        const archi_krcvlist_t *node = (archi_krcvlist_t*)archi_kvlist_find(context->ptr, slot.name, NULL, NULL);
        if (node == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "key '%s' isn't in the list", slot.name);
            return;
        }

        ARCHI_CONTEXT_YIELD(node->value);
    }
    else
    {
        if (ARCHI_STRING_COMPARE("", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            archi_krcvlist_t *copy = archi_context_parameters_copy(params, context->ptr,
                    ARCHI_ERROR_PARAM);
            if ((copy == NULL) && (params != NULL))
                return;

            // Update context pointer and attributes
            context->ptr = copy;
            context->attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__KRCVLIST);
        }
        else
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown call '%s' encountered", slot.name);
            return;
        }

        ARCHI_ERROR_RESET();
    }
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__parameters)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

    if (slot.num_indices != 0)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
        return;
    }

    // Increment reference counter of the new value or copy its memory
    value = archi_rcpointer_own(value, ARCHI_ERROR_PARAM);
    if (!value.attr)
        return;

    // Find node with the specified key
    archi_krcvlist_t *node = (archi_krcvlist_t*)archi_kvlist_find(context->ptr, slot.name, NULL, NULL);

    if (node == NULL)
    {
        // Create new node
        node = (archi_krcvlist_t*)archi_kvlist_node_alloc(slot.name, true);
        if (node == NULL)
        {
            archi_rcpointer_disown(value);

            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate key-value list node");
            return;
        }

        node->next = context->ptr;
        node->value = value;

        // Update context pointer and attributes
        context->ptr = node;
        context->attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__KRCVLIST);
    }
    else
    {
        archi_rcpointer_disown(node->value);
        node->value = value;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__parameters = {
    .init_fn = archi_context_init__parameters,
    .final_fn = archi_context_final__parameters,
    .eval_fn = archi_context_eval__parameters,
    .set_fn = archi_context_set__parameters,
};

