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
 * @brief Branch selector functions for loops in directed execution graphs.
 */

#include "archi/deg/deg/loop.fun.h"
#include "archi/deg/deg/loop.typ.h"

ARCHI_DEG_BRANCH_FUNCTION(archi_deg_branch__loop_for)
{
    archi_deg_branch_data__loop_for_t *loop = data;
    if (loop == NULL)
        return 1; // break loop

    if (loop->iteration_idx < loop->num_iterations) // proceed
    {
        loop->iteration_idx++;
        return 0;
    }
    else // break loop
    {
        loop->iteration_idx = 0;
        return 1;
    }
}

