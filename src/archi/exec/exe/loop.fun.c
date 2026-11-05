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
 * @brief Transition functions for loops in directed execution graphs.
 */

#include "archi/exec/exe/loop.fun.h"
#include "archi/exec/exe/loop.typ.h"


ARCHI_DEXGRAPH_CONDITION_FUNC(archi_dexgraph_condition__loop_times)
{
    archi_dexgraph_condition_data__loop_times_t *loop = data;

    if (loop == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "loop state is NULL");
        return false;
    }

    ARCHI_ERROR_RESET();

    return (loop->iteration < loop->num_iterations)
        ? (loop->iteration++, false)   // do another iteration
        : (loop->iteration = 0, true); // break the loop
}

