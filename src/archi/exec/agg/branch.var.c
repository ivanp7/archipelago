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
 * @brief Aggregate type descriptions for data of transition functions for branching.
 */

#include "archi/exec/agg/branch.var.h"
#include "archi/exec/agg/graph.var.h"
#include "archi/exec/exe/branch.typ.h"
#include "archi/exec/exe/tag.def.h"
#include "archi/exec/i/tag.def.h"


static
const archi_aggr_member_type__pointer_t
PTYPE_data = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(void*, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_dexgraph_condition_func = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_FUNC(archi_dexgraph_condition_func_t,
        ARCHI_POINTER_FUNC_TAG__DEXGRAPH_CONDITION);

static
const archi_aggr_member_type__pointer_t
PTYPE_dexgraph_node = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(struct archi_dexgraph_node*,
        ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_transition_data__branch[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_transition_data__branch_t, function, 1, PTYPE_dexgraph_condition_func),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_transition_data__branch_t, data, 1, PTYPE_data),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_transition_data__branch_t, target, 2, PTYPE_dexgraph_node),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_transition_data__branch = ARCHI_AGGR_TYPE(
        archi_dexgraph_transition_data__branch_t,
        ARCHI_POINTER_DATA_TAG__DEXGRAPH_TRANSITION_DATA__BRANCH,
        MEMBERS_dexgraph_transition_data__branch);

