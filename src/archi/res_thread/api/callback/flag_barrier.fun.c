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
 * @brief Thread group callbacks for flag barriers.
 */

#include "archi/res_thread/api/callback/flag_barrier.fun.h"
#include "archi/res_thread/api/flag_barrier.fun.h"
#include "archipelago/log/print.fun.h"

ARCHI_THREAD_GROUP_CALLBACK_FUNC(archi_thread_group_callback_flag_barrier_release)
{
    (void) work_size;
    (void) thread_idx;

    archi_status_t code = archi_thread_flag_barrier_release(data);

    if (code != 0)
        archi_log_error(__func__, "archi_thread_flag_barrier_release() -> %i", code);
}

