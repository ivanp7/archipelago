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
 * @brief HSP state for thread group dispatch operation.
 */

#include "archi/thread/hsp/dispatch.fun.h"
#include "archi/thread/hsp/dispatch.typ.h"
#include "archi/thread/api/thread_group.fun.h"

ARCHI_HSP_STATE_FUNCTION(archi_hsp_state__thread_group_dispatch)
{
    (void) hsp;

    archi_hsp_state_data__thread_group_dispatch_t *dispatch_data = data;
    if (dispatch_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "thread group dispatch data is NULL");
        return;
    }

    archi_error_t error;

    for (;;)
    {
        // Attempt dispatch
        bool success = archi_thread_group_dispatch(dispatch_data->thread_group,
                dispatch_data->work, dispatch_data->callback, dispatch_data->param, &error);

        if (success || (error.code != 0))
            break;

        // Busy: wait and retry
        archi_thread_group_wait(dispatch_data->thread_group);
    }

    ARCHI_ERROR_ASSIGN(error);
}

ARCHI_HSP_STATE_FUNCTION(archi_hsp_state__thread_group_wait)
{
    (void) hsp;

    if (data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "thread group is NULL");
        return;
    }

    archi_thread_group_wait(data);

    ARCHI_ERROR_RESET();
}

