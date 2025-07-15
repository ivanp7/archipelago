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

#pragma once
#ifndef _ARCHI_UTIL_TIMER_FUN_H_
#define _ARCHI_UTIL_TIMER_FUN_H_

#include <stdbool.h>

struct archi_timer;

/**
 * @brief Pointer to timer context.
 */
typedef struct archi_timer *archi_timer_t;

/**
 * @brief Allocate a timer.
 *
 * @return Timer context.
 */
archi_timer_t
archi_timer_alloc(void);

/**
 * @brief Destroy a timer.
 */
void
archi_timer_free(
        archi_timer_t timer ///< [in] Timer.
);

/**
 * @brief Reset a timer.
 */
void
archi_timer_reset(
        archi_timer_t timer ///< [in] Timer.
);

/**
 * @brief Start a timer.
 *
 * @return True if the timer has been started, false if the timer was already in the started state.
 */
bool
archi_timer_start(
        archi_timer_t timer ///< [in] Timer.
);

/**
 * @brief Stop a timer.
 *
 * @return Number of seconds passed since the timer start.
 */
float
archi_timer_stop(
        archi_timer_t timer ///< [in] Timer.
);

/**
 * @brief Total number of runs of a timer.
 *
 * @return Total number of timer stops.
 */
unsigned long
archi_timer_runs_done(
        archi_timer_t timer ///< [in] Timer.
);

/**
 * @brief Total accumulated time of a timer.
 *
 * @return Total number of seconds passed between all starts and stops.
 */
float
archi_timer_time_total(
        archi_timer_t timer ///< [in] Timer.
);

/**
 * @brief Average time of a timer.
 *
 * @return Average number of seconds passed between all starts and stops.
 */
float
archi_timer_time_average(
        archi_timer_t timer ///< [in] Timer.
);

/**
 * @brief Minimum time of a timer.
 *
 * @return Minimum number of seconds passed between all starts and stops.
 */
float
archi_timer_time_minimum(
        archi_timer_t timer ///< [in] Timer.
);

/**
 * @brief Maximum time of a timer.
 *
 * @return Maximum number of seconds passed between all starts and stops.
 */
float
archi_timer_time_maximum(
        archi_timer_t timer ///< [in] Timer.
);

/**
 * @brief Last time of a timer.
 *
 * @return Last number of seconds passed between start and stop.
 */
float
archi_timer_time_last(
        archi_timer_t timer ///< [in] Timer.
);

#endif // _ARCHI_UTIL_TIMER_FUN_H_

