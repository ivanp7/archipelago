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
 * @brief Context interface for directed execution graph operation sequences.
 */

#include "archi/exec/ctx/operation.var.h"
#include "archi/exec/i/operation.fun.h"
#include "archi/exec/i/tag.def.h"
#include "archi/context/i/interface.def.h"
#include "archi_base/rcpointer.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/tag.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"


struct archi_context_data__dexgraph_operation_seq {
    archi_rcpointer_t sequence;

    // References
    archi_rcpointer_t *ref_function;
    archi_rcpointer_t *ref_data;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__dexgraph_operation_seq)
{
    // Parse parameters
    size_t sequence_length = 0;
    {
        archi_plist_param_t parsed[] = {
            {.name = "length",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &sequence_length, sizeof(sequence_length), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Construct the context
    struct archi_context_data__dexgraph_operation_seq *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__dexgraph_operation_seq){
        .sequence = {
            .ptr = archi_dexgraph_operation_seq_alloc(sequence_length),
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_OPERATION_SEQ),
        },
    };

    if (context_data->sequence.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate DEG operation sequence (length = %zu)",
                sequence_length);
        goto failure;
    }

    // Initialize references
    if (sequence_length != 0)
    {
        context_data->ref_function = malloc(sizeof(*context_data->ref_function) * sequence_length);
        if (context_data->ref_function == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to operation functions (length = %zu)",
                    sequence_length);
            goto failure;
        }

        context_data->ref_data = malloc(sizeof(*context_data->ref_data) * sequence_length);
        if (context_data->ref_data == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references to data of operation functions (length = %zu)",
                    sequence_length);
            goto failure;
        }

        for (size_t i = 0; i < sequence_length; i++)
        {
            context_data->ref_function[i] = (archi_rcpointer_t){0};
            context_data->ref_data[i] = (archi_rcpointer_t){0};
        }
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    archi_dexgraph_operation_seq_free(context_data->sequence.ptr);

    free(context_data->ref_function);
    free(context_data->ref_data);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__dexgraph_operation_seq)
{
    struct archi_context_data__dexgraph_operation_seq *context_data =
        (struct archi_context_data__dexgraph_operation_seq*)context;

    archi_dexgraph_operation_seq_t *sequence = context_data->sequence.ptr;

    for (size_t i = 0; i < sequence->length; i++)
    {
        archi_rcpointer_disown(context_data->ref_function[i]);
        archi_rcpointer_disown(context_data->ref_data[i]);
    }

    archi_dexgraph_operation_seq_free(sequence);

    free(context_data->ref_function);
    free(context_data->ref_data);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__dexgraph_operation_seq)
{
    struct archi_context_data__dexgraph_operation_seq *context_data =
        (struct archi_context_data__dexgraph_operation_seq*)context;

    archi_dexgraph_operation_seq_t *sequence = context_data->sequence.ptr;

    if (!call)
    {
        if (ARCHI_STRING_COMPARE("length", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t sequence_length = sequence->length;

            archi_rcpointer_t value = {
                .ptr = &sequence_length,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, size_t),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("function", ==, slot.name))
        {
            if (slot.num_indices != 1)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
                return;
            }

            archi_context_slot_index_t index = slot.index[0];
            if ((index < 0) || ((size_t)index >= sequence->length))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                        index, sequence->length);
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_function[index]);
        }
        else if (ARCHI_STRING_COMPARE("data", ==, slot.name))
        {
            if (slot.num_indices != 1)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
                return;
            }

            archi_context_slot_index_t index = slot.index[0];
            if ((index < 0) || ((size_t)index >= sequence->length))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                        index, sequence->length);
                return;
            }

            ARCHI_CONTEXT_YIELD(context_data->ref_data[index]);
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

            // Execute the operation sequence
            archi_dexgraph_operation_seq_exec(sequence, 0, ARCHI_ERROR_PARAM);
        }
        else
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
    }
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__dexgraph_operation_seq)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

    struct archi_context_data__dexgraph_operation_seq *context_data =
        (struct archi_context_data__dexgraph_operation_seq*)context;

    archi_dexgraph_operation_seq_t *sequence = context_data->sequence.ptr;

    if (ARCHI_STRING_COMPARE("function", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__func(ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION), NULL))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a DEG operation function");
            return;
        }

        archi_context_slot_index_t index = slot.index[0];
        if ((index < 0) || ((size_t)index >= sequence->length))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                    index, sequence->length);
            return;
        }

        value = archi_rcpointer_replace(value, context_data->ref_function[index],
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        sequence->operation[index].function = (archi_dexgraph_operation_func_t)value.fptr;
        context_data->ref_function[index] = value;

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("data", ==, slot.name))
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
        if ((index < 0) || ((size_t)index >= sequence->length))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index %lli is out of bounds [0; %zu)",
                    index, sequence->length);
            return;
        }

        value = archi_rcpointer_replace(value, context_data->ref_data[index],
                ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        sequence->operation[index].data = value.p;
        context_data->ref_data[index] = value;

        ARCHI_ERROR_RESET();
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }
}

const archi_context_interface_t
archi_context_interface__dexgraph_operation_seq = {
    .init_fn = archi_context_init__dexgraph_operation_seq,
    .final_fn = archi_context_final__dexgraph_operation_seq,
    .eval_fn = archi_context_eval__dexgraph_operation_seq,
    .set_fn = archi_context_set__dexgraph_operation_seq,
};

