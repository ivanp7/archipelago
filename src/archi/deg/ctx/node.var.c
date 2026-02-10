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

#include "archi/deg/ctx/node.var.h"
#include "archi/deg/api/node.fun.h"
#include "archi/deg/api/graph.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

struct archi_context_data__deg_node {
    archi_rcpointer_t node;

    // References
    archi_rcpointer_t ref_default_branch;
    archi_rcpointer_t ref_alt_branches;

    archi_rcpointer_t ref_branch_func;
    archi_rcpointer_t ref_branch_func_data;

    archi_rcpointer_t *ref_sequence_func;
    archi_rcpointer_t *ref_sequence_func_data;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__deg_node)
{
    // Parse parameters
    size_t sequence_length = 0;
    archi_rcpointer_t def_branch = {0};
    archi_rcpointer_t alt_branches = {0};
    archi_rcpointer_t branch_func = {0};
    archi_rcpointer_t branch_func_data = {0};
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "sequence_length", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "default_branch", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_deg_node_t)},
            {.name = "alt_branches", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(0, archi_deg_node_t*)},
            {.name = "branch_function", .value.attr = archi_pointer_attr__function(ARCHI_POINTER_FUNCTION_TAG__DEG_BRANCH)},
            {.name = "branch_func_data", .value.attr = archi_pointer_attr__opaque_data(0)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            sequence_length = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            def_branch = parsed[index].value;
        index++;
        if (parsed[index].value_set)
            alt_branches = parsed[index].value;
        index++;
        if (parsed[index].value_set)
            branch_func = parsed[index].value;
        index++;
        if (parsed[index].value_set)
            branch_func_data = parsed[index].value;
    }

    // Construct the context
    struct archi_context_data__deg_node *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__deg_node){
        .node = {
            .ptr = archi_deg_node_alloc(sequence_length, NULL, 0, NULL, NULL),
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_deg_node_t),
        },
    };

    if (context_data->node.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate DEG node (sequence length = %zu)",
                sequence_length);
        goto failure;
    }

    if (sequence_length != 0)
    {
        context_data->ref_sequence_func = malloc(sizeof(*context_data->ref_sequence_func) * sequence_length);
        if (context_data->ref_sequence_func == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to functions (length = %zu)",
                    sequence_length);
            goto failure;
        }

        context_data->ref_sequence_func_data = malloc(sizeof(*context_data->ref_sequence_func_data) * sequence_length);
        if (context_data->ref_sequence_func_data == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to data (length = %zu)",
                    sequence_length);
            goto failure;
        }

        for (size_t i = 0; i < sequence_length; i++)
        {
            context_data->ref_sequence_func[i] = (archi_rcpointer_t){0};
            context_data->ref_sequence_func_data[i] = (archi_rcpointer_t){0};
        }
    }

    context_data->ref_default_branch = archi_rcpointer_own(def_branch, ARCHI_ERROR_PARAMETER);
    if (!context_data->ref_default_branch.attr) // failed to own
        goto failure;

    context_data->ref_alt_branches = archi_rcpointer_own(alt_branches, ARCHI_ERROR_PARAMETER);
    if (!context_data->ref_alt_branches.attr) // failed to own
        goto failure;

    context_data->ref_branch_func = archi_rcpointer_own(branch_func, ARCHI_ERROR_PARAMETER);
    if (!context_data->ref_branch_func.attr) // failed to own
        goto failure;

    context_data->ref_branch_func_data = archi_rcpointer_own(branch_func_data, ARCHI_ERROR_PARAMETER);
    if (!context_data->ref_branch_func_data.attr) // failed to own
        goto failure;

    archi_deg_node_t *deg_node = context_data->node.ptr;

    deg_node->def_branch = context_data->ref_default_branch.ptr;
    archi_pointer_attr_parse__transp_data(context_data->ref_alt_branches.attr,
            &deg_node->num_alt_branches, NULL, NULL, NULL);
    deg_node->alt_branch = context_data->ref_alt_branches.ptr;
    deg_node->branch_selector.function = (archi_deg_branch_function_t)context_data->ref_branch_func.fptr;
    deg_node->branch_selector.data = context_data->ref_branch_func_data.ptr;

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    archi_rcpointer_disown(context_data->ref_default_branch);
    archi_rcpointer_disown(context_data->ref_alt_branches);
    archi_rcpointer_disown(context_data->ref_branch_func);
    archi_rcpointer_disown(context_data->ref_branch_func_data);

    archi_deg_node_free(context_data->node.ptr);

    free(context_data->ref_sequence_func);
    free(context_data->ref_sequence_func_data);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__deg_node)
{
    struct archi_context_data__deg_node *context_data =
        (struct archi_context_data__deg_node*)context;

    archi_deg_node_t *deg_node = context_data->node.ptr;

    for (size_t i = 0; i < deg_node->sequence_length; i++)
    {
        archi_rcpointer_disown(context_data->ref_sequence_func[i]);
        archi_rcpointer_disown(context_data->ref_sequence_func_data[i]);
    }

    archi_rcpointer_disown(context_data->ref_default_branch);
    archi_rcpointer_disown(context_data->ref_alt_branches);
    archi_rcpointer_disown(context_data->ref_branch_func);
    archi_rcpointer_disown(context_data->ref_branch_func_data);

    deg_node->alt_branch = NULL; // array is allocated elsewhere and must not be deallocated here
    archi_deg_node_free(deg_node);

    free(context_data->ref_sequence_func);
    free(context_data->ref_sequence_func_data);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__deg_node)
{
    struct archi_context_data__deg_node *context_data =
        (struct archi_context_data__deg_node*)context;

    archi_deg_node_t *deg_node = context_data->node.ptr;

    if (!call)
    {
        if (ARCHI_STRING_COMPARE("sequence.length", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t sequence_length = deg_node->sequence_length;

            archi_rcpointer_t value = {
                .ptr = &sequence_length,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
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

            ptrdiff_t index = slot.index[0];
            if ((index < 0) || ((size_t)index >= deg_node->sequence_length))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu)",
                        index, deg_node->sequence_length);
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_sequence_func[index]);
        }
        else if (ARCHI_STRING_COMPARE("sequence.func_data", ==, slot.name))
        {
            if (slot.num_indices != 1)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
                return;
            }

            ptrdiff_t index = slot.index[0];
            if ((index < 0) || ((size_t)index >= deg_node->sequence_length))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu)",
                        index, deg_node->sequence_length);
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_sequence_func_data[index]);
        }
        else if (ARCHI_STRING_COMPARE("branch.default", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_default_branch);
        }
        else if (ARCHI_STRING_COMPARE("branch.alternatives", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_alt_branches);
        }
        else if (ARCHI_STRING_COMPARE("branch.num_alternatives", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t num_alt_branches = deg_node->num_alt_branches;

            archi_rcpointer_t value = {
                .ptr = &num_alt_branches,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("branch.function", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_branch_func);
        }
        else if (ARCHI_STRING_COMPARE("branch.func_data", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_branch_func_data);
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

            if (params != NULL)
            {
                ARCHI_ERROR_SET(ARCHI__EKEY, "no parameters are accepted");
                return;
            }

            archi_deg_execute(deg_node, ARCHI_ERROR_PARAMETER);
        }
        else
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
    }
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__deg_node)
{
    struct archi_context_data__deg_node *context_data =
        (struct archi_context_data__deg_node*)context;

    archi_deg_node_t *deg_node = context_data->node.ptr;

    if (ARCHI_STRING_COMPARE("sequence.function", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__function(ARCHI_POINTER_FUNCTION_TAG__DEG_NODE)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned function is not a DEG node function");
            return;
        }

        ptrdiff_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= deg_node->sequence_length))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu)",
                    index, deg_node->sequence_length);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_sequence_func[index],
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        deg_node->sequence[index].function = (archi_deg_node_function_t)value.fptr;
        context_data->ref_sequence_func[index] = value;
    }
    else if (ARCHI_STRING_COMPARE("sequence.func_data", ==, slot.name))
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

        ptrdiff_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= deg_node->sequence_length))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu)",
                    index, deg_node->sequence_length);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_sequence_func_data[index],
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        deg_node->sequence[index].data = value.ptr;
        context_data->ref_sequence_func_data[index] = value;
    }
    else if (ARCHI_STRING_COMPARE("branch.default", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_deg_node_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a DEG node");
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_default_branch,
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        deg_node->def_branch = value.ptr;
        context_data->ref_default_branch = value;
    }
    else if (ARCHI_STRING_COMPARE("branch.alternatives", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(0, archi_deg_node_t*)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not an array of DEG node pointers");
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_alt_branches,
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        archi_pointer_attr_parse__transp_data(value.attr, &deg_node->num_alt_branches, NULL, NULL, NULL);
        deg_node->alt_branch = value.ptr;
        context_data->ref_alt_branches = value;
    }
    else if (ARCHI_STRING_COMPARE("branch.function", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__function(ARCHI_POINTER_FUNCTION_TAG__DEG_BRANCH)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned function is not a DEG branch function");
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_branch_func,
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        deg_node->branch_selector.function = (archi_deg_branch_function_t)value.fptr;
        context_data->ref_branch_func = value;
    }
    else if (ARCHI_STRING_COMPARE("branch.func_data", ==, slot.name))
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

        value = archi_rcpointer_own_disown(value, context_data->ref_branch_func_data,
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        deg_node->branch_selector.data = value.ptr;
        context_data->ref_branch_func_data = value;
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__deg_node = {
    .init_fn = archi_context_init__deg_node,
    .final_fn = archi_context_final__deg_node,
    .eval_fn = archi_context_eval__deg_node,
    .set_fn = archi_context_set__deg_node,
};

