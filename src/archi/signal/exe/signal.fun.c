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
 * @brief Transition functions for signaled conditions.
 */

#include "archi/signal/exe/signal.fun.h"
#include "archi/signal/exe/signal.typ.h"
#include "archi/signal/api/signal.fun.h"
#include "archi_base/util/size.def.h"

#include <stdlib.h> // for malloc()


archi_dexgraph_transition_data__signal_detect_t*
archi_dexgraph_transition_data__signal_detect_alloc(
        size_t num_signals)
{
    archi_dexgraph_transition_data__signal_detect_t *detect = malloc(ARCHI_SIZEOF_FLEXIBLE(
                archi_dexgraph_transition_data__signal_detect_t, signal, num_signals));
    if (detect == NULL)
        return NULL;

    detect->flags = NULL;
    {
        size_t *num_signals_ptr = (size_t*)&detect->num_signals;
        *num_signals_ptr = num_signals;
    }
    for (size_t i = 0; i < num_signals; i++)
        detect->signal[i] = (archi_dexgraph_transition_data__signal_detect__signal_t){.index = i};

    return detect;
}

ARCHI_DEXGRAPH_TRANSITION_FUNC(archi_dexgraph_transition__signal_detect)
{
    archi_dexgraph_transition_data__signal_detect_t *detect = data;

    if (detect == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data of signal detection transition function is NULL");
        return ARCHI_DEXGRAPH_HALT;
    }
    else if (detect->flags == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "signal flags is NULL");
        return ARCHI_DEXGRAPH_HALT;
    }

    const archi_signal_flags_t *flags = detect->flags;

    for (size_t i = 0; i < detect->num_signals; i++)
    {
        int signal_index = detect->signal[i].index;

        if ((signal_index < 0) || (signal_index >= ARCHI_SIGNAL_NUMBER + ARCHI_SIGNAL_NUMBER_REALTIME))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "invalid signal index %i encountered", signal_index);
            return ARCHI_DEXGRAPH_HALT;
        }

        if (signal_index < ARCHI_SIGNAL_NUMBER)
        {
            if (ARCHI_SIGNAL_FLAG_IS_SET(flags->signal[signal_index]))
            {
                ARCHI_ERROR_RESET();
                return detect->signal[i].branch_index;
            }
        }
        else
        {
            if (ARCHI_SIGNAL_FLAG_IS_SET(flags->rt_signal[signal_index - ARCHI__SIGRTMIN]))
            {
                ARCHI_ERROR_RESET();
                return detect->signal[i].branch_index;
            }
        }
    }

    ARCHI_ERROR_RESET();
    return 0;
}

