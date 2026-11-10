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
 * @brief Functions of directed execution graph operations.
 */

#include "archi/exec/i/operation.fun.h"
#include "archi/exec/i/tag.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/util/size.def.h"

#include <stdlib.h> // for malloc(), free()


void
archi_dexgraph_operation_call(
        archi_dexgraph_operation_t operation,
        ARCHI_ERROR_PARAM_DECL)
{
    if (operation.function != NULL)
    {
        ARCHI_ERROR_UNSET();
        /****************************************************/
        operation.function(operation.data, ARCHI_ERROR_PARAM);
        /****************************************************/
    }
    else
    {
        if (!archi_pointer_attr_compatible(operation.data.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_OPERATION_SEQ), NULL))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "operation data is not an operation sequence");
            return;
        }

        archi_dexgraph_operation_seq_exec(operation.data.cptr, 0, ARCHI_ERROR_PARAM);
    }
}

/*****************************************************************************/

archi_dexgraph_operation_seq_t*
archi_dexgraph_operation_seq_alloc(
        size_t length)
{
    archi_dexgraph_operation_seq_t *sequence = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archi_dexgraph_operation_seq_t, operation, length));
    if (sequence == NULL)
        return NULL;

    sequence->name = NULL;
    {
        size_t *length_ptr = (size_t*)&sequence->length;
        *length_ptr = length;
    }
    for (size_t i = 0; i < length; i++)
        sequence->operation[i] = (archi_dexgraph_operation_t){0};

    return sequence;
}

void
archi_dexgraph_operation_seq_free(
        archi_dexgraph_operation_seq_t *sequence)
{
    free(sequence);
}

archi_dexgraph_operation_t
archi_dexgraph_operation_seq_elt(
        const archi_dexgraph_operation_seq_t *sequence,
        size_t op_index)
{
    if ((sequence == NULL) || (op_index >= sequence->length))
        return (archi_dexgraph_operation_t){0};

    return sequence->operation[op_index];
}

size_t
archi_dexgraph_operation_seq_exec(
        const archi_dexgraph_operation_seq_t *sequence,
        size_t op_index,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((sequence == NULL) || (sequence->length == 0))
    {
        ARCHI_ERROR_RESET();
        return ARCHI_DEXGRAPH_OPERATION_SEQ_END;
    }

    archi_error_t error;

    for (; op_index < sequence->length; op_index++)
    {
        archi_dexgraph_operation_call(sequence->operation[op_index], &error);

        if (error.code != 0)
            goto finish;
    }
    op_index = ARCHI_DEXGRAPH_OPERATION_SEQ_END;

finish:
    ARCHI_ERROR_ASSIGN(error);
    return op_index;
}

