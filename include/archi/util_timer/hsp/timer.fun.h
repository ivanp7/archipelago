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

#pragma once
#ifndef _ARCHI_UTIL_TIMER_HSP_TIMER_FUN_H_
#define _ARCHI_UTIL_TIMER_HSP_TIMER_FUN_H_

#include "archi/hsp/api/transition.typ.h"

/**
 * @brief Profiling timer reset function.
 */
ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition_timer_reset);

/**
 * @brief Profiling timer start function.
 */
ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition_timer_start);

/**
 * @brief Profiling timer stop function.
 */
ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition_timer_stop);

/**
 * @brief Profiling timer report function.
 */
ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition_timer_report);

#endif // _ARCHI_UTIL_TIMER_HSP_TIMER_FUN_H_

