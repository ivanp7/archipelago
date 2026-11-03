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
 * @brief Transition functions for branch index mapping in directed execution graphs.
 */

#include "archi/exec/exe/mapped.fun.h"
#include "archi/exec/exe/mapped.typ.h"
#include "archi/exec/i/transition.fun.h"


ARCHI_DEXGRAPH_TRANSITION_FUNC(archi_dexgraph_transition__mapped)
{
    const archi_dexgraph_transition_data__mapped_t *mapped = data;

    if (mapped == NULL)
    {
        ARCHI_ERROR_RESET();
        return 0;
    }

    size_t branch_index = archi_dexgraph_transition_execute(mapped->transition, ARCHI_ERROR_PARAM);
    return (branch_index < mapped->table_length) ? mapped->table[branch_index] : ARCHI_DEXGRAPH_HALT;
}

