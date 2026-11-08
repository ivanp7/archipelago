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
 * @brief Operation functions for executing directed execution graphs.
 */

#include "archi/exec/exe/graph.fun.h"
#include "archi/exec/i/frame.fun.h"
#include "archi/exec/i/tag.def.h"
#include "archi_base/pointer.fun.h"


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__execute)
{
    if (!archi_pointer_attr_compatible(data.attr,
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "operation function data has incorrect type");
        return;
    }

    archi_dexgraph_frame_t frame = {.node = data.cptr};
    archi_dexgraph_execute(frame, ARCHI_ERROR_PARAM);
}

ARCHI_DEXGRAPH_TRANSITION_FUNC(archi_dexgraph_transition__execute_node)
{
    if (!archi_pointer_attr_compatible(data.attr,
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "transition function data has incorrect type");
        return NULL;
    }

    archi_error_t error;

    archi_dexgraph_frame_t frame = {.node = data.cptr};
    const struct archi_dexgraph_node *target =
        archi_dexgraph_execute_node(frame, &error).node;

    ARCHI_ERROR_ASSIGN(error);
    return (error.code == 0) ? target : NULL;
}

