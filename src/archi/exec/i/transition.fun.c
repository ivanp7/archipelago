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
#include "archi/exec/i/tag.def.h"
#include "archi_base/pointer.fun.h"


const struct archi_dexgraph_node*
archi_dexgraph_transition_target(
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
        if (!archi_pointer_attr_compatible(transition.data.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "transition data is not a node");
            return NULL;
        }

        ARCHI_ERROR_RESET();
        return transition.data.cptr;
    }
}

