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

#pragma once
#ifndef _ARCHI_TIMER_EXE_TIMER_FUN_H_
#define _ARCHI_TIMER_EXE_TIMER_FUN_H_

#include "archi/exec/api/operation.typ.h"


/**
 * @brief Operation function: reset a profiling timer.
 *
 * Function data type: archi_timer_t.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__timer_reset);

/**
 * @brief Operation function: start a profiling timer.
 *
 * Function data type: archi_timer_t.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__timer_start);

/**
 * @brief Operation function: stop a profiling timer.
 *
 * Function data type: archi_timer_t.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__timer_stop);

/**
 * @brief Operation function: print report of profiling timer state.
 *
 * Function data type: archi_timer_t.
 *
 * The report is printed to the standard output stream.
 */
ARCHI_DEXGRAPH_OPERATION_FUNC(archi_dexgraph_op__timer_report);

#endif // _ARCHI_TIMER_EXE_TIMER_FUN_H_

