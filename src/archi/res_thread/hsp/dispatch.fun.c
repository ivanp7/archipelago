/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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

#include "archi/res_thread/hsp/dispatch.fun.h"
#include "archi/res_thread/hsp/dispatch.typ.h"
#include "archi/res_thread/api/thread_group.fun.h"
#include "archi/hsp/api/state.fun.h"
#include "archipelago/log/print.fun.h"

ARCHI_HSP_STATE_FUNCTION(archi_hsp_state_thread_group_dispatch)
{
    archi_thread_group_dispatch_data_t *dispatch_data = ARCHI_HSP_CURRENT_STATE().data;
    if ((dispatch_data == NULL) || (dispatch_data->work == NULL))
        return;

    archi_thread_group_callback_t callback = {0};
    if (dispatch_data->callback != NULL)
        callback = *dispatch_data->callback;

    archi_status_t code = archi_thread_group_dispatch(dispatch_data->context,
            *dispatch_data->work, callback, dispatch_data->params);

    if (code != 0)
    {
        if (dispatch_data->name != NULL)
            archi_log_error(__func__, "archi_thread_group_dispatch('%s') -> %i", dispatch_data->name, code);
        else
            archi_log_error(__func__, "archi_thread_group_dispatch() -> %i", code);
    }
}

