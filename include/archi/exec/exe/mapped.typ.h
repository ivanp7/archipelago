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
 * @brief Data for transition functions for branch index mapping in directed execution graphs.
 */

#pragma once
#ifndef _ARCHI_EXEC_EXE_MAPPED_TYP_H_
#define _ARCHI_EXEC_EXE_MAPPED_TYP_H_

#include "archi/exec/i/transition.typ.h"


/**
 * @brief Transition function data: map branch index according to the table.
 *
 * The mapping is done by fetching the new index from the table at the old index:
 * `new_index = old_index < table_length ? table[old_index] : ARCHI_DEXGRAPH_HALT`
 */
typedef struct archi_dexgraph_transition_data__mapped {
    archi_dexgraph_transition_t transition; ///< Transition.

    const size_t table_length; ///< Branch index table length.
    size_t table[]; ///< Branch index table.
} archi_dexgraph_transition_data__mapped_t;

#endif // _ARCHI_EXEC_EXE_MAPPED_TYP_H_

