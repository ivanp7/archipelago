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
 * @brief Aggregate type descriptions for directed execution graph nodes and transitions.
 */

#include "archi/exec/agg/graph.var.h"
#include "archi/exec/api/node.typ.h"
#include "archi/exec/api/transition.typ.h"
#include "archi/exec/api/tag.def.h"


static
const archi_aggr_member_type__value_t
VTYPE_size = ARCHI_AGGR_MEMBER_TYPE__VALUE(size_t, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_string = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_PDATA(char*, char, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_data = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(void*, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_dexgraph_node = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(archi_dexgraph_node_t*,
        ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE);

static
const archi_aggr_member_type__pointer_t
PTYPE_dexgraph_node_array = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(archi_dexgraph_node_array_t*,
        ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY);

static
const archi_aggr_member_type__pointer_t
PTYPE_dexgraph_operation_func = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_FUNC(archi_dexgraph_operation_func_t,
        ARCHI_POINTER_FUNC_TAG__DEXGRAPH_OPERATION);

static
const archi_aggr_member_type__pointer_t
PTYPE_dexgraph_transition_func = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_FUNC(archi_dexgraph_transition_func_t,
        ARCHI_POINTER_FUNC_TAG__DEXGRAPH_TRANSITION);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_transition[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_transition_t, function, 1, PTYPE_dexgraph_transition_func),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_transition_t, data, 1, PTYPE_data),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_transition = ARCHI_AGGR_TYPE(
        archi_dexgraph_transition_t, 0,
        MEMBERS_dexgraph_transition);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_operation[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_operation_t, function, 1, PTYPE_dexgraph_operation_func),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_operation_t, data, 1, PTYPE_data),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_operation = ARCHI_AGGR_TYPE(
        archi_dexgraph_operation_t, 0,
        MEMBERS_dexgraph_operation);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_node[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_node_t, name, 1, PTYPE_string),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_node_t, transition, 1,
            archi_aggr_type__dexgraph_transition.top_level),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_node_t, branch, 1, PTYPE_dexgraph_node_array),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_node_t, sequence_length, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_node_t, sequence, 0,
            archi_aggr_type__dexgraph_operation.top_level),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_node = ARCHI_AGGR_TYPE(
        archi_dexgraph_node_t, ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE,
        MEMBERS_dexgraph_node);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_node_array[] = {
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_node_array_t, num_nodes, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_node_array_t, node, 0, PTYPE_dexgraph_node),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_node_array = ARCHI_AGGR_TYPE(
        archi_dexgraph_node_array_t, ARCHI_POINTER_DATA_TAG__DEXGRAPH_NODE_ARRAY,
        MEMBERS_dexgraph_node_array);

