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
#include "archi/exec/api/node.fun.h"
#include "archi/exec/api/graph.fun.h"
#include "archi/exec/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/tag.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"


struct archi_context_data__dexgraph_node {
    archi_rcpointer_t node;

    // References
    archi_rcpointer_t ref_transition_func;
    archi_rcpointer_t ref_transition_data;

    archi_rcpointer_t ref_branch_array;

    archi_rcpointer_t *ref_sequence_func;
    archi_rcpointer_t *ref_sequence_data;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__dexgraph_node)
{
    // Parse parameters
    const char *name = NULL;
    size_t sequence_length = 0;
    archi_rcpointer_t transition_func = {0}, transition_data = {0}, branch_array = {0};
    {
        archi_plist_param_t parsed[] = {
            {.name = "name",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(0, char)}},
                .assign = {archi_plist_assign__dptr_n, &name, sizeof(name)}},
            {.name = "sequence_length",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &sequence_length, sizeof(sequence_length)}},
            {.name = "transition_func",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__func(ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION)}},
                .assign = {archi_plist_assign__rcpointer, &transition_func, sizeof(transition_func)}},
            {.name = "transition_data",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                .assign = {archi_plist_assign__rcpointer, &transition_data, sizeof(transition_data)}},
            {.name = "branches",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)}},
                .assign = {archi_plist_assign__rcpointer, &branch_array, sizeof(branch_array)}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Construct the context
    struct archi_context_data__dexgraph_node *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__dexgraph_node){
        .node = {
            .ptr = archi_dexgraph_node_alloc(name, sequence_length),
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE),
        },
    };

    if (context_data->node.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate DEG node (sequence length = %zu)",
                sequence_length);
        goto failure;
    }

    // Initialize references
    if (sequence_length != 0)
    {
        context_data->ref_sequence_func = malloc(sizeof(*context_data->ref_sequence_func) * sequence_length);
        if (context_data->ref_sequence_func == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to operation functions (length = %zu)",
                    sequence_length);
            goto failure;
        }

        context_data->ref_sequence_data = malloc(sizeof(*context_data->ref_sequence_data) * sequence_length);
        if (context_data->ref_sequence_data == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to data of operation functions (length = %zu)",
                    sequence_length);
            goto failure;
        }

        for (size_t i = 0; i < sequence_length; i++)
        {
            context_data->ref_sequence_func[i] = (archi_rcpointer_t){0};
            context_data->ref_sequence_data[i] = (archi_rcpointer_t){0};
        }
    }

    context_data->ref_transition_func = archi_rcpointer_own(transition_func, ARCHI_ERROR_PARAM);
    if (!context_data->ref_transition_func.attr)
        goto failure;

    context_data->ref_transition_data = archi_rcpointer_own(transition_data, ARCHI_ERROR_PARAM);
    if (!context_data->ref_transition_data.attr)
        goto failure;

    context_data->ref_branch_array = archi_rcpointer_own(branch_array, ARCHI_ERROR_PARAM);
    if (!context_data->ref_branch_array.attr)
        goto failure;

    // Initialize data
    archi_dexgraph_node_t *node = context_data->node.ptr;

    node->transition.function = (archi_dexgraph_transition_func_t)context_data->ref_transition_func.fptr;
    node->transition.data = context_data->ref_transition_data.ptr;
    node->branch = context_data->ref_branch_array.cptr;

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    archi_rcpointer_disown(context_data->ref_transition_func);
    archi_rcpointer_disown(context_data->ref_transition_data);
    archi_rcpointer_disown(context_data->ref_branch_array);

    archi_dexgraph_node_free(context_data->node.ptr);

    free(context_data->ref_sequence_func);
    free(context_data->ref_sequence_data);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__dexgraph_node)
{
    struct archi_context_data__dexgraph_node *context_data =
        (struct archi_context_data__dexgraph_node*)context;

    archi_dexgraph_node_t *node = context_data->node.ptr;

    for (size_t i = 0; i < node->sequence_length; i++)
    {
        archi_rcpointer_disown(context_data->ref_sequence_func[i]);
        archi_rcpointer_disown(context_data->ref_sequence_data[i]);
    }

    archi_rcpointer_disown(context_data->ref_transition_func);
    archi_rcpointer_disown(context_data->ref_transition_data);
    archi_rcpointer_disown(context_data->ref_branch_array);

    archi_dexgraph_node_free(node);

    free(context_data->ref_sequence_func);
    free(context_data->ref_sequence_data);
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

            archi_rcpointer_t value = {
                .ptr = node->name,
                .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                    ARCHI_POINTER_ATTR__PDATA((node->name != NULL) ? strlen(node->name) + 1 : 0, char),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("sequence.length", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t sequence_length = node->sequence_length;

            archi_rcpointer_t value = {
                .ptr = &sequence_length,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, size_t),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("sequence.function", ==, slot.name))
        {
            if (slot.num_indices != 1)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
                return;
            }

            archi_context_slot_index_t index = slot.index[0];
            if ((index < 0) || ((size_t)index >= node->sequence_length))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                        index, node->sequence_length);
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_sequence_func[index]);
        }
        else if (ARCHI_STRING_COMPARE("sequence.data", ==, slot.name))
        {
            if (slot.num_indices != 1)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
                return;
            }

            archi_context_slot_index_t index = slot.index[0];
            if ((index < 0) || ((size_t)index >= node->sequence_length))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                        index, node->sequence_length);
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_sequence_data[index]);
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
        else if (ARCHI_STRING_COMPARE("branches", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_branch_array);
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

            // Parse parameters
            size_t index = 0;
            {
                archi_plist_param_t parsed[] = {
                    {.name = "index",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_dexgraph_branch_index_t)}},
                        .assign = {archi_plist_assign__value, &index, sizeof(index)}},
                    {0},
                };

                if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
                    return;
            }

            // Execute the graph
            archi_dexgraph_frame_t frame = {
                .node = node,
                .index = index,
            };

            archi_dexgraph_execute(frame, ARCHI_DEXGRAPH__NO_INTERRUPT, ARCHI_ERROR_PARAM);
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

    if (ARCHI_STRING_COMPARE("sequence.function", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__func(ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a DEG operation function");
            return;
        }

        archi_context_slot_index_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= node->sequence_length))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                    index, node->sequence_length);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_sequence_func[index],
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        node->sequence[index].function = (archi_dexgraph_operation_func_t)value.fptr;
        context_data->ref_sequence_func[index] = value;

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("sequence.data", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not data");
            return;
        }

        archi_context_slot_index_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= node->sequence_length))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                    index, node->sequence_length);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_sequence_data[index],
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        node->sequence[index].data = value.ptr;
        context_data->ref_sequence_data[index] = value;

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
                    archi_pointer_attr__func(ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a DEG transition function");
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_transition_func,
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

        value = archi_rcpointer_own_disown(value, context_data->ref_transition_data,
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        node->transition.data = value.ptr;
        context_data->ref_transition_data = value;

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("branches", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a DEG node array");
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_branch_array,
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        node->branch = value.cptr;
        context_data->ref_branch_array = value;

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

/*****************************************************************************/

struct archi_context_data__dexgraph_node_array {
    archi_rcpointer_t node_array;

    // References
    archi_rcpointer_t *ref_node;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__dexgraph_node_array)
{
    // Parse parameters
    size_t num_nodes = 0;
    {
        archi_plist_param_t parsed[] = {
            {.name = "num_nodes",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &num_nodes, sizeof(num_nodes)}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Construct the context
    struct archi_context_data__dexgraph_node_array *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__dexgraph_node_array){
        .node_array = {
            .ptr = archi_dexgraph_node_array_alloc(num_nodes),
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY),
        },
    };

    if (context_data->node_array.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate DEG node array (length = %zu)", num_nodes);
        goto failure;
    }

    // Initialize references
    if (num_nodes != 0)
    {
        context_data->ref_node = malloc(sizeof(*context_data->ref_node) * num_nodes);
        if (context_data->ref_node == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to nodes (length = %zu)",
                    num_nodes);
            goto failure;
        }

        for (size_t i = 0; i < num_nodes; i++)
            context_data->ref_node[i] = (archi_rcpointer_t){0};
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    archi_dexgraph_node_array_free(context_data->node_array.ptr);

    free(context_data->ref_node);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__dexgraph_node_array)
{
    struct archi_context_data__dexgraph_node_array *context_data =
        (struct archi_context_data__dexgraph_node_array*)context;

    archi_dexgraph_node_array_t *node_array = context_data->node_array.ptr;

    for (size_t i = 0; i < node_array->num_nodes; i++)
        archi_rcpointer_disown(context_data->ref_node[i]);

    archi_dexgraph_node_array_free(node_array);

    free(context_data->ref_node);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__dexgraph_node_array)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__dexgraph_node_array *context_data =
        (struct archi_context_data__dexgraph_node_array*)context;

    archi_dexgraph_node_array_t *node_array = context_data->node_array.ptr;

    if (ARCHI_STRING_COMPARE("num_nodes", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t num_nodes = node_array->num_nodes;

        archi_rcpointer_t value = {
            .ptr = &num_nodes,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("node", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }

        archi_context_slot_index_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= node_array->num_nodes))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                    index, node_array->num_nodes);
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->ref_node[index]);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__dexgraph_node_array)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

    struct archi_context_data__dexgraph_node_array *context_data =
        (struct archi_context_data__dexgraph_node_array*)context;

    archi_dexgraph_node_array_t *node_array = context_data->node_array.ptr;

    if (ARCHI_STRING_COMPARE("node", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a DEG node");
            return;
        }

        archi_context_slot_index_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= node_array->num_nodes))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                    index, node_array->num_nodes);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_node[index],
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        node_array->node[index] = value.cptr;
        context_data->ref_node[index] = value;

        ARCHI_ERROR_RESET();
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_interface__dexgraph_node_array = {
    .init_fn = archi_context_init__dexgraph_node_array,
    .final_fn = archi_context_final__dexgraph_node_array,
    .eval_fn = archi_context_eval__dexgraph_node_array,
    .set_fn = archi_context_set__dexgraph_node_array,
};

