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
    else if (!archi_print_lock(0))
        return;

    archi_print("\n------ TIMER REPORT ------\n");

    {
        const char *name = archi_timer_name(data);
        if (name != NULL)
            archi_print("Name         : %s\n", name);
    }

    {
        unsigned long runs_done = archi_timer_runs_done(data);
        archi_print("Runs done    : %lu\n", runs_done);
    }

#define PRINT_TIME(name, func) { \
        float time = func(data); \
        archi_print(name ": %.3g s (%.3g ms)\n", time, time * 1e+3f); \
    }

    PRINT_TIME("Minimum time ", archi_timer_time_minimum);
    PRINT_TIME("Average time ", archi_timer_time_average);
    PRINT_TIME("Maximum time ", archi_timer_time_maximum);
    PRINT_TIME("Last time    ", archi_timer_time_last);

#undef PRINT_TIME

    {
        float runs_per_second = 1.0f / archi_timer_time_average(data);
        archi_print("Average rate : %.3g runs/s (%.3g runs/ms)\n", runs_per_second, runs_per_second * 1e-3f);
    }

    archi_print("--- END OF TIMER REPORT ---\n");

    archi_print_unlock();
}

