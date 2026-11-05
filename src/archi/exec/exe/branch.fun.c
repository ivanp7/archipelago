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
 * @brief Transition functions for branching in directed execution graphs.
 */

#include "archi/exec/exe/branch.fun.h"
#include "archi/exec/exe/branch.typ.h"


ARCHI_DEXGRAPH_TRANSITION_FUNC(archi_dexgraph_transition__branch)
{
    const archi_dexgraph_transition_data__branch_t *branch = data;

    if (branch == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "branch description is NULL");
        return NULL;
    }
    else if (branch->function == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "branch condition function is NULL");
        return NULL;
    }

    archi_error_t error;
    ARCHI_ERROR_VAR_UNSET(&error);

    /******************************************************/
    bool condition = branch->function(branch->data, &error);
    /******************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        return NULL;

    return branch->target[condition];
}

