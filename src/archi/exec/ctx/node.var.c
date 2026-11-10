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
 * @brief Context interface for directed execution graph nodes.
 */

#include "archi/exec/ctx/node.var.h"
#include "archi/exec/i/node.typ.h"
#include "archi/exec/i/node.fun.h"
#include "archi/exec/i/tag.def.h"
#include "archi/context/i/interface.def.h"
#include "archi_base/rcpointer.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/tag.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"


struct archi_context_data__dexgraph_node {
    archi_rcpointer_t node;

    // References
    archi_rcpointer_t ref_name;
    archi_rcpointer_t ref_sequence;
    archi_rcpointer_t ref_transition_func;
    archi_rcpointer_t ref_transition_data;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__dexgraph_node)
{
    // Parse parameters
    archi_rcpointer_t name = {0}, sequence = {0}, transition_func = {0}, transition_data = {0};
    {
        archi_plist_param_t parsed[] = {
            {.name = "name",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(0, char)}},
                .assign = {archi_plist_assign__rcpointer, &name, sizeof(name), NULL}},
            {.name = "sequence",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_OPERATION_SEQ)}},
                .assign = {archi_plist_assign__rcpointer, &sequence, sizeof(sequence), NULL}},
            {.name = "transition_func",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__func(ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION)}},
                .assign = {archi_plist_assign__rcpointer, &transition_func, sizeof(transition_func), NULL}},
            {.name = "transition_data",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                .assign = {archi_plist_assign__rcpointer, &transition_data, sizeof(transition_data), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    ARCHI_POINTER_NULLIFY_EMPTY(name);

    // Construct the context
    struct archi_context_data__dexgraph_node *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__dexgraph_node){
        .node = {
            .ptr = malloc(sizeof(archi_dexgraph_node_t)),
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE),
        },
    };

    if (context_data->node.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate DEG node");
        goto failure;
    }

    // Initialize references
    context_data->ref_name = archi_rcpointer_own(name, ARCHI_ERROR_PARAM);
    if (!context_data->ref_name.attr)
        goto failure;

    context_data->ref_sequence = archi_rcpointer_own(sequence, ARCHI_ERROR_PARAM);
    if (!context_data->ref_sequence.attr)
        goto failure;

    context_data->ref_transition_func = archi_rcpointer_own(transition_func, ARCHI_ERROR_PARAM);
    if (!context_data->ref_transition_func.attr)
        goto failure;

    context_data->ref_transition_data = archi_rcpointer_own(transition_data, ARCHI_ERROR_PARAM);
    if (!context_data->ref_transition_data.attr)
        goto failure;

    // Initialize data
    archi_dexgraph_node_t *node = context_data->node.ptr;

    node->name = context_data->ref_name.cptr;
    node->sequence = context_data->ref_sequence.cptr;
    node->transition.function = (archi_dexgraph_transition_func_t)context_data->ref_transition_func.fptr;
    node->transition.data = context_data->ref_transition_data.p;

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    archi_rcpointer_disown(context_data->ref_name);
    archi_rcpointer_disown(context_data->ref_sequence);
    archi_rcpointer_disown(context_data->ref_transition_func);
    archi_rcpointer_disown(context_data->ref_transition_data);

    free(context_data->node.ptr);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__dexgraph_node)
{
    struct archi_context_data__dexgraph_node *context_data =
        (struct archi_context_data__dexgraph_node*)context;

    archi_rcpointer_disown(context_data->ref_name);
    archi_rcpointer_disown(context_data->ref_sequence);
    archi_rcpointer_disown(context_data->ref_transition_func);
    archi_rcpointer_disown(context_data->ref_transition_data);

    free(context_data->node.ptr);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__dexgraph_node)
{
    struct archi_context_data__dexgraph_node *context_data =
        (struct archi_context_data__dexgraph_node*)context;

    archi_dexgraph_node_t *node = context_data->node.ptr;

    if (!call)
    {
        if (ARCHI_STRING_COMPARE("name", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_name);
        }
        else if (ARCHI_STRING_COMPARE("sequence", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_sequence);
        }
        else if (ARCHI_STRING_COMPARE("transition.function", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_transition_func);
        }
        else if (ARCHI_STRING_COMPARE("transition.data", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_transition_data);
        }
        else
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
    }
    else
    {
        if (ARCHI_STRING_COMPARE("execute", ==, slot.name))
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

            // Execute the graph
            archi_dexgraph_execute(node, ARCHI_ERROR_PARAM);
        }
        else
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
    }
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__dexgraph_node)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

    struct archi_context_data__dexgraph_node *context_data =
        (struct archi_context_data__dexgraph_node*)context;

    archi_dexgraph_node_t *node = context_data->node.ptr;

    if (ARCHI_STRING_COMPARE("name", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__PDATA(0, char), NULL))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a string");
            return;
        }

        ARCHI_POINTER_NULLIFY_EMPTY(value);

        value = archi_rcpointer_replace(value, context_data->ref_name,
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        node->name = value.cptr;
        context_data->ref_name = value;

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("sequence", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_OPERATION_SEQ), NULL))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not an DEG operation sequence");
            return;
        }

        value = archi_rcpointer_replace(value, context_data->ref_sequence,
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        node->sequence = value.cptr;
        context_data->ref_sequence = value;

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("transition.function", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__func(ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION), NULL))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a DEG transition function");
            return;
        }

        value = archi_rcpointer_replace(value, context_data->ref_transition_func,
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        node->transition.function = (archi_dexgraph_transition_func_t)value.fptr;
        context_data->ref_transition_func = value;

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("transition.data", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not data");
            return;
        }

        value = archi_rcpointer_replace(value, context_data->ref_transition_data,
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        node->transition.data = value.p;
        context_data->ref_transition_data = value;

        ARCHI_ERROR_RESET();
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }
}

const archi_context_interface_t
archi_context_interface__dexgraph_node = {
    .init_fn = archi_context_init__dexgraph_node,
    .final_fn = archi_context_final__dexgraph_node,
    .eval_fn = archi_context_eval__dexgraph_node,
    .set_fn = archi_context_set__dexgraph_node,
};

