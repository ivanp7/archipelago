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
 * @brief Aggregate type descriptions for data of transition functions for loops.
 */

#include "archi/exec/agg/loop.var.h"
#include "archi/exec/exe/loop.typ.h"


static
const archi_aggr_member_type__value_t
VTYPE_size = ARCHI_AGGR_MEMBER_TYPE__VALUE(size_t, 0);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_transition_data__loop_times[] = {
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_transition_data__loop_times_t, iteration, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_transition_data__loop_times_t, num_iterations, 1, VTYPE_size),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_transition_data__loop_times = ARCHI_AGGR_TYPE(
        archi_dexgraph_transition_data__loop_times_t, MEMBERS_dexgraph_transition_data__loop_times);

