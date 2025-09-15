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
 * @brief HSP transition functions for profiling timer.
 */

#include "archi/util_timer/hsp/timer.fun.h"
#include "archi/util_timer/api/timer.fun.h"
#include "archipelago/log/print.fun.h"

ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition_timer_reset)
{
    (void) prev_state;
    (void) next_state;
    (void) trans_state;

    archi_timer_reset(data);
}

ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition_timer_start)
{
    (void) prev_state;
    (void) next_state;
    (void) trans_state;

    archi_timer_start(data);
}

ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition_timer_stop)
{
    (void) prev_state;
    (void) next_state;
    (void) trans_state;

    archi_timer_stop(data);
}

ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition_timer_report)
{
    (void) prev_state;
    (void) next_state;
    (void) trans_state;

    if (data == NULL)
        return;

    archi_log_debug(archi_timer_name(data),
            "last = %.3g, min = %.3g, max = %.3g, average = %.3g, runs-done = %lu, per-second = %.3g",
            archi_timer_time_last(data),
            archi_timer_time_minimum(data),
            archi_timer_time_maximum(data),
            archi_timer_time_average(data),
            archi_timer_runs_done(data),
            1.0f / archi_timer_time_average(data));
}

