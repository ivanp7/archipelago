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
 * @brief Functions of directed execution graph transitions.
 */

#include "archi/exec/i/transition.fun.h"


size_t
archi_dexgraph_transition_execute(
        archi_dexgraph_transition_t transition,
        ARCHI_ERROR_PARAM_DECL)
{
    if (transition.function != NULL)
    {
        ARCHI_ERROR_UNSET();
        /*************************************************************/
        return transition.function(transition.data, ARCHI_ERROR_PARAM);
        /*************************************************************/
    }
    else
    {
        ARCHI_ERROR_RESET();
        return (transition.data != NULL) ? *(size_t*)transition.data : 0;
    }
}

