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
 * @brief Arrays of hierarchical state processor transitions.
 */

#include "archi/hsp/hsp/array_transition.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h> // for malloc()

archi_hsp_transition_data__array_t*
archi_hsp_transition_data_alloc__array(
        size_t num_transitions)
{
    archi_hsp_transition_data__array_t *transition_data = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archi_hsp_transition_data__array_t, transition, num_transitions));
    if (transition_data == NULL)
        return NULL;

    size_t *num_transitions_ptr = (size_t*)&transition_data->num_transitions;
    *num_transitions_ptr = num_transitions;

    for (size_t i = 0; i < num_transitions; i++)
        transition_data->transition[i] = NULL;

    return transition_data;
}

/*****************************************************************************/

ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition__array)
{
    const archi_hsp_transition_data__array_t *transition_data = data;
    if (transition_data == NULL)
        return;

    for (size_t i = 0; i < transition_data->num_transitions; i++)
    {
        if ((transition_data->transition[i] == NULL) ||
                (transition_data->transition[i]->function == NULL))
            continue;

        transition_data->transition[i]->function(prev_state, next_state,
                transition_data->transition[i]->data, ARCHI_ERROR_PARAMETER);
    }
}

