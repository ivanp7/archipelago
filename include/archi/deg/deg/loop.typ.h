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
 * @brief Data for branch selector functions for loops in directed execution graphs.
 */

#pragma once
#ifndef _ARCHI_DEG_DEG_LOOP_TYP_H_
#define _ARCHI_DEG_DEG_LOOP_TYP_H_

#include <stddef.h> // for size_t

/**
 * @brief Branch selector function data: for-loop.
 */
typedef union archi_deg_branch_data__loop_for {
    struct {
        size_t num_iterations; ///< Number of iterations to do.
        size_t iteration_idx;  ///< Current iteration index.
    };
    size_t field[2]; ///< Structure as array.
} archi_deg_branch_data__loop_for_t;

#endif // _ARCHI_DEG_DEG_LOOP_TYP_H_

