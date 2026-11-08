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
 * @brief DEG operation functions for thread group operations.
 */

#include "archi/thread/exe/thread_group.fun.h"
#include "archi/thread/exe/thread_group.typ.h"
#include "archi/thread/exe/tag.def.h"
#include "archi/thread/i/thread_group.fun.h"
#include "archi/thread/i/tag.def.h"
#include "archi_base/pointer.fun.h"


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__thread_group_dispatch)
{
    if (!archi_pointer_attr_compatible(data.attr,
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_OP_DATA__THREAD_GROUP_DISPATCH)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "operation function data has incorrect type");
        return;
    }

    const archi_dexgraph_op_data__thread_group_dispatch_t *dispatch_data = data.cptr;

    if (dispatch_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "thread group dispatch operation parameters is NULL");
        return;
    }

    // Dispatch the work to the thread group
    archi_error_t error;

    for (;;)
    {
        // Make an attempt
        ARCHI_ERROR_VAR_UNSET(&error);
        bool success = archi_thread_group_dispatch(dispatch_data->thread_group,
                dispatch_data->work, dispatch_data->callback, dispatch_data->param, &error);

        if (success || (error.code != 0))
            break;

        // Busy: wait and retry
        archi_thread_group_wait(dispatch_data->thread_group);
    }

    ARCHI_ERROR_ASSIGN(error);
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__thread_group_wait)
{
    if (!archi_pointer_attr_compatible(data.attr,
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__THREAD_GROUP)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "operation function data has incorrect type");
        return;
    }

    archi_thread_group_t thread_group = data.ptr;

    if (thread_group == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "thread group context is NULL");
        return;
    }

    archi_thread_group_wait(thread_group);

    ARCHI_ERROR_RESET();
}

