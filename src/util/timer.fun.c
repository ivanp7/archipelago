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
 * @brief Profiling timer interface.
 */

#include "archi/util/timer.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strlen(), memcpy()
#include <math.h> // for INFINITY
#include <sys/time.h> // for gettimeofday(), struct timeval

struct archi_timer {
    const char *name; ///< Timer name.

    float total_seconds; ///< Total accumulated time.
    float min_seconds;   ///< Minimum run time.
    float max_seconds;   ///< Maximum run time.
    float last_seconds;  ///< Last run time.

    unsigned long runs_done; ///< Number of runs done.

    struct timeval start_time; ///< Timer start time.
    bool started; ///< Whether the timer is started.
};

archi_timer_t
archi_timer_alloc(
        const char *name)
{
    archi_timer_t timer = malloc(sizeof(*timer));
    if (timer == NULL)
        return NULL;

    if (name != NULL)
    {
        size_t name_len = strlen(name) + 1;

        char *name_copy = malloc(name_len);
        if (name_copy == NULL)
        {
            free(timer);
            return NULL;
        }

        memcpy(name_copy, name, name_len);
        name = name_copy;
    }

    timer->name = name;
    archi_timer_reset(timer);

    return timer;
}

void
archi_timer_free(
        archi_timer_t timer)
{
    free(timer->name);
    free(timer);
}

void
archi_timer_reset(
        archi_timer_t timer)
{
    if (timer == NULL)
        return;

    const char *name = timer->name;

    *timer = (struct archi_timer){
        .name = name,
    };
}

bool
archi_timer_start(
        archi_timer_t timer)
{
    if ((timer == NULL) || timer->started)
        return false;

    int ret = gettimeofday(&timer->start_time, NULL);
    if (ret != 0)
        return false;

    timer->started = true;
    return true;
}

float
archi_timer_stop(
        archi_timer_t timer)
{
    if ((timer == NULL) || !timer->started)
        return -INFINITY;

    struct timeval stop_time;
    int ret = gettimeofday(&stop_time, NULL);
    if (ret != 0)
        return -INFINITY;

    if (timer->start_time.tv_sec > stop_time.tv_sec)
        return -INFINITY;

    float seconds;
    {
        seconds = stop_time.tv_sec - timer->start_time.tv_sec;
        seconds += ((long)stop_time.tv_usec  - (long)timer->start_time.tv_usec) * 1e-6f;
    }

    if (seconds < 0.0f)
        return -INFINITY;

    timer->last_seconds = seconds;
    timer->total_seconds += seconds;

    if (timer->runs_done > 0)
    {
        if (seconds < timer->min_seconds)
            timer->min_seconds = seconds;

        if (seconds > timer->max_seconds)
            timer->max_seconds = seconds;
    }
    else
        timer->min_seconds = timer->max_seconds = seconds;

    timer->runs_done++;

    timer->started = false;
    return seconds;
}

const char*
archi_timer_name(
        archi_timer_t timer)
{
    if (timer == NULL)
        return NULL;

    return timer->name;
}

unsigned long
archi_timer_runs_done(
        archi_timer_t timer)
{
    if (timer == NULL)
        return 0;

    return timer->runs_done;
}

float
archi_timer_time_total(
        archi_timer_t timer)
{
    if (timer == NULL)
        return 0.0f;

    return timer->total_seconds;
}

float
archi_timer_time_average(
        archi_timer_t timer)
{
    if ((timer == NULL) || (timer->runs_done == 0))
        return 0.0f;

    return timer->total_seconds / timer->runs_done;
}

float
archi_timer_time_minimum(
        archi_timer_t timer)
{
    if (timer == NULL)
        return 0.0f;

    return timer->min_seconds;
}

float
archi_timer_time_maximum(
        archi_timer_t timer)
{
    if (timer == NULL)
        return 0.0f;

    return timer->max_seconds;
}

float
archi_timer_time_last(
        archi_timer_t timer)
{
    if (timer == NULL)
        return 0.0f;

    return timer->last_seconds;
}

