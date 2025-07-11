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
 * @brief Hierarchical state processor states for thread groups.
 */

#include "archi/builtin/res_thread_group_hsp/state.fun.h"
#include "archi/builtin/res_thread_group/dispatch.typ.h"
#include "archi/res/thread_group/api.fun.h"
#include "archi/hsp/state.fun.h"
#include "archi/log/print.fun.h"

ARCHI_HSP_STATE_FUNCTION(archi_hsp_state_res_thread_group_dispatch)
{
#define M "archi_hsp_state_res_thread_group_dispatch"

    archi_context_res_thread_group_dispatch_data_t *dispatch_data = ARCHI_HSP_CURRENT_STATE().data;
    if (dispatch_data == NULL)
        return;

    archi_status_t code = archi_thread_group_dispatch(dispatch_data->context,
            dispatch_data->work, dispatch_data->callback, dispatch_data->params);

    if (code != 0)
    {
        if (dispatch_data->name != NULL)
            archi_log_error(M, "archi_thread_group_dispatch('%s') -> %i", dispatch_data->name, code);
        else
            archi_log_error(M, "archi_thread_group_dispatch() -> %i", code);
    }

#undef M
}

