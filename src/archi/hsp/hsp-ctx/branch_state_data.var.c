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
 * @brief Context interface for HSP branch state data.
 */

#include "archi/hsp/hsp-ctx/branch_state_data.var.h"
#include "archi/hsp/hsp/branch_state.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

struct archi_context_data__hsp_state_data__branch {
    archi_rcpointer_t state_data;

    // References
    archi_rcpointer_t ref_selector_fn;
    archi_rcpointer_t ref_selector_data;
    archi_rcpointer_t *ref_frame;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__hsp_state_data__branch)
{
    // Parse parameters
    size_t num_branches = 0;
    archi_rcpointer_t selector_fn = {0};
    archi_rcpointer_t selector_data = {0};
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "num_branches", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "selector_function", .value.attr =
                archi_pointer_attr__function(ARCHI_POINTER_FUNCTION_TAG__HSP_BRANCH_SELECTOR)},
            {.name = "selector_data", .value.attr = archi_pointer_attr__opaque_data(0)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            num_branches = *(size_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            selector_fn = parsed[index].value;
        index++;
        if (parsed[index].value_set)
            selector_data = parsed[index].value;
    }

    // Check validity of parameters
    if (num_branches >= (ARCHI_POINTER_DATA_SIZE_MAX + 1) / sizeof(archi_rcpointer_t))
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "number of branches is too big (%zu)", num_branches);
        return NULL;
    }

    // Construct the context
    struct archi_context_data__hsp_state_data__branch *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    archi_hsp_state_data__branch_t *branch_state_data = archi_hsp_state_data_alloc__branch(
            num_branches, (archi_hsp_branch_selector_func_t)selector_fn.fptr, selector_data.ptr);
    if (branch_state_data == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate branch state data");
        return NULL;
    }

    *context_data = (struct archi_context_data__hsp_state_data__branch){
        .state_data = {
            .ptr = branch_state_data,
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_hsp_state_data__branch_t),
        },
    };

    if (num_branches != 0)
    {
        context_data->ref_frame = malloc(sizeof(*context_data->ref_frame) * num_branches);
        if (context_data->ref_frame == NULL)
        {
            free(branch_state_data);
            free(context_data);

            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to branch frames [%zu]",
                    num_branches);
            return NULL;
        }

        for (size_t i = 0; i < num_branches; i++)
            context_data->ref_frame[i] = (archi_rcpointer_t){0};
    }

    context_data->ref_selector_fn = archi_rcpointer_own(selector_fn, ARCHI_ERROR_PARAMETER);
    if (!context_data->ref_selector_fn.attr)
    {
        free(context_data->ref_frame);
        free(branch_state_data);
        free(context_data);

        return NULL;
    }

    context_data->ref_selector_data = archi_rcpointer_own(selector_data, ARCHI_ERROR_PARAMETER);
    if (!context_data->ref_selector_data.attr)
    {
        free(context_data->ref_frame);
        free(branch_state_data);
        free(context_data);

        return NULL;
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__hsp_state_data__branch)
{
    struct archi_context_data__hsp_state_data__branch *context_data =
        (struct archi_context_data__hsp_state_data__branch*)context;

    archi_hsp_state_data__branch_t *branch_state_data = context_data->state_data.ptr;

    for (size_t i = 0; i < branch_state_data->num_branches; i++)
        archi_rcpointer_disown(context_data->ref_frame[i]);

    archi_rcpointer_disown(context_data->ref_selector_fn);
    archi_rcpointer_disown(context_data->ref_selector_data);

    free(context_data->ref_frame);
    free(context_data->state_data.ptr);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__hsp_state_data__branch)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__hsp_state_data__branch *context_data =
        (struct archi_context_data__hsp_state_data__branch*)context;

    archi_hsp_state_data__branch_t *branch_state_data = context_data->state_data.ptr;

    if (ARCHI_STRING_COMPARE("num_branches", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t num_branches = branch_state_data->num_branches;

        archi_rcpointer_t value = {
            .ptr = &num_branches,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("branch", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }

        ptrdiff_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= branch_state_data->num_branches))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu branches)",
                    index, branch_state_data->num_branches);
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->ref_frame[index]);
    }
    else if (ARCHI_STRING_COMPARE("selector.function", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->ref_selector_fn);
    }
    else if (ARCHI_STRING_COMPARE("selector.data", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->ref_selector_data);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__hsp_state_data__branch)
{
    struct archi_context_data__hsp_state_data__branch *context_data =
        (struct archi_context_data__hsp_state_data__branch*)context;

    archi_hsp_state_data__branch_t *branch_state_data = context_data->state_data.ptr;

    if (ARCHI_STRING_COMPARE("branch", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(value.ptr != NULL, archi_hsp_frame_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a HSP frame");
            return;
        }
        else if (ARCHI_POINTER_TO_STACK(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "HSP branch frame is on the stack");
            return;
        }

        ptrdiff_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= branch_state_data->num_branches))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%ti) out of bounds (%zu branches)",
                    index, branch_state_data->num_branches);
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_frame[index],
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        branch_state_data->branch[index] = value.ptr;
        context_data->ref_frame[index] = value;
    }
    else if (ARCHI_STRING_COMPARE("selector.function", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__function(ARCHI_POINTER_FUNCTION_TAG__HSP_BRANCH_SELECTOR)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned function is not a HSP branch selector function");
            return;
        }

        value = archi_rcpointer_own_disown(value, context_data->ref_selector_fn,
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        branch_state_data->selector_fn = (archi_hsp_branch_selector_func_t)value.fptr;
        context_data->ref_selector_fn = value;
    }
    else if (ARCHI_STRING_COMPARE("selector.data", ==, slot.name))
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

        value = archi_rcpointer_own_disown(value, context_data->ref_selector_data,
                ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        branch_state_data->selector_data = value.ptr;
        context_data->ref_selector_data = value;
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__hsp_state_data__branch = {
    .init_fn = archi_context_init__hsp_state_data__branch,
    .final_fn = archi_context_final__hsp_state_data__branch,
    .eval_fn = archi_context_eval__hsp_state_data__branch,
    .set_fn = archi_context_set__hsp_state_data__branch,
};

