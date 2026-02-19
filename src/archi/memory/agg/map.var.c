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
 * @brief Aggregate type descriptions for data of operation functions for memory mapping/unmapping.
 */

#include "archi/memory/agg/map.var.h"
#include "archi/exec/agg/graph.var.h"
#include "archi/memory/exe/map.typ.h"
#include "archi/memory/api/tag.def.h"
#include "archi/exec/api/tag.def.h"


static
const archi_aggr_member_type__value_t
VTYPE_size = ARCHI_AGGR_MEMBER_TYPE__VALUE(size_t, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_data = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(void*, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_ptr_to_ptr = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_PDATA(void**, void*, 1);

static
const archi_aggr_member_type__pointer_t
PTYPE_memory = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(archi_memory_t,
        ARCHI_POINTER_DATA_TAG__MEMORY);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_node_data__memory_map_unmap_wrapper[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__memory_map_unmap_wrapper_t, memory, 1, PTYPE_memory),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__memory_map_unmap_wrapper_t, map_data, 1, PTYPE_data),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_op_data__memory_map_unmap_wrapper_t, map_offset, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_op_data__memory_map_unmap_wrapper_t, map_length, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_op_data__memory_map_unmap_wrapper_t, target, 1,
            archi_aggr_type__dexgraph_operation.top_level),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__memory_map_unmap_wrapper_t, ptr, 1, PTYPE_ptr_to_ptr),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_node_data__memory_map_unmap_wrapper = ARCHI_AGGR_TYPE(
        archi_dexgraph_op_data__memory_map_unmap_wrapper_t, MEMBERS_dexgraph_node_data__memory_map_unmap_wrapper);

