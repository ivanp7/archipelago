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
 * @brief DEG operation functions for profiling timer operations.
 */

#include "archi/timer/exe/timer.fun.h"
#include "archi/timer/i/timer.fun.h"
#include "archi/timer/i/tag.def.h"
#include "archi_base/pointer.fun.h"

#include <stdio.h>


ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__timer_reset)
{
    {
        archi_error_t error;
        if (!archi_pointer_attr_compatible(data.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__TIMER),
                    &error))
        {
            ARCHI_ERROR_SET(error.code, "operation data has incorrect type: %s", error.message);
            return;
        }
    }

    archi_timer_t timer = data.ptr;

    if (timer == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "timer is NULL");
        return;
    }

    archi_timer_reset(timer);

    ARCHI_ERROR_RESET();
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__timer_start)
{
    {
        archi_error_t error;
        if (!archi_pointer_attr_compatible(data.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__TIMER),
                    &error))
        {
            ARCHI_ERROR_SET(error.code, "operation data has incorrect type: %s", error.message);
            return;
        }
    }

    archi_timer_t timer = data.ptr;

    if (timer == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "timer is NULL");
        return;
    }

    archi_timer_start(timer);

    ARCHI_ERROR_RESET();
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__timer_stop)
{
    {
        archi_error_t error;
        if (!archi_pointer_attr_compatible(data.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__TIMER),
                    &error))
        {
            ARCHI_ERROR_SET(error.code, "operation data has incorrect type: %s", error.message);
            return;
        }
    }

    archi_timer_t timer = data.ptr;

    if (timer == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "timer is NULL");
        return;
    }

    archi_timer_stop(timer);

    ARCHI_ERROR_RESET();
}

ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__timer_report)
{
    {
        archi_error_t error;
        if (!archi_pointer_attr_compatible(data.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__TIMER),
                    &error))
        {
            ARCHI_ERROR_SET(error.code, "operation data has incorrect type: %s", error.message);
            return;
        }
    }

    archi_timer_t timer = data.ptr;

    if (timer == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "timer is NULL");
        return;
    }

    printf("\n------ TIMER REPORT ------\n");

    {
        const char *name = archi_timer_name(timer);
        if (name != NULL)
            printf("Name         : %s\n", name);
    }

    {
        unsigned long runs_done = archi_timer_runs_done(timer);
        printf("Runs done    : %lu\n", runs_done);
    }

#define PRINT_TIME(name, func) {                                    \
        float time = func(timer);                                   \
        printf(name ": %.3g s (%.3g ms)\n", time, time * 1e+3f);    \
    }

    PRINT_TIME("Minimum time ", archi_timer_time_minimum);
    PRINT_TIME("Average time ", archi_timer_time_average);
    PRINT_TIME("Maximum time ", archi_timer_time_maximum);
    PRINT_TIME("Last time    ", archi_timer_time_last);

#undef PRINT_TIME

    {
        float runs_per_second = 1.0f / archi_timer_time_average(timer);
        printf("Average rate : %.3g runs/s (%.3g runs/ms)\n",
                runs_per_second, runs_per_second * 1e-3f);
    }

    printf("--- END OF TIMER REPORT ---\n");

    ARCHI_ERROR_RESET();
}

