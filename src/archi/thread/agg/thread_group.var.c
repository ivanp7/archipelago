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
 * @brief Aggregate type descriptions for data of operation functions for thread group operations.
 */

#include "archi/thread/agg/thread_group.var.h"
#include "archi/thread/exe/thread_group.typ.h"
#include "archi/thread/api/tag.def.h"


static
const archi_aggr_member_type__value_t
VTYPE_size = ARCHI_AGGR_MEMBER_TYPE__VALUE(size_t, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_data = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(void*, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_thread_group_work_func = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_FUNC(archi_thread_group_work_func_t,
        ARCHI_POINTER_FUNC_TAG__THREAD_WORK);

static
const archi_aggr_member_type__pointer_t
PTYPE_thread_group_callback_func = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_FUNC(archi_thread_group_callback_func_t,
        ARCHI_POINTER_FUNC_TAG__THREAD_CALLBACK);

static
const archi_aggr_member_type__pointer_t
PTYPE_thread_group = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(archi_thread_group_t,
        ARCHI_POINTER_DATA_TAG__THREAD_GROUP);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_thread_group_work[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_thread_group_work_t, function, 1, PTYPE_thread_group_work_func),
    ARCHI_AGGR_MEMBER__POINTER(archi_thread_group_work_t, data, 1, PTYPE_data),
};

const archi_aggr_type_t
archi_aggr_type__thread_group_work = ARCHI_AGGR_TYPE(
        archi_thread_group_work_t, MEMBERS_thread_group_work);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_thread_group_callback[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_thread_group_callback_t, function, 1, PTYPE_thread_group_callback_func),
    ARCHI_AGGR_MEMBER__POINTER(archi_thread_group_callback_t, data, 1, PTYPE_data),
};

const archi_aggr_type_t
archi_aggr_type__thread_group_callback = ARCHI_AGGR_TYPE(
        archi_thread_group_callback_t, MEMBERS_thread_group_callback);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_thread_group_dispatch_params[] = {
    ARCHI_AGGR_MEMBER__VALUE(archi_thread_group_dispatch_params_t, offset, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__VALUE(archi_thread_group_dispatch_params_t, size, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__VALUE(archi_thread_group_dispatch_params_t, batch_size, 1, VTYPE_size),
};

const archi_aggr_type_t
archi_aggr_type__thread_group_dispatch_params = ARCHI_AGGR_TYPE(
        archi_thread_group_dispatch_params_t, MEMBERS_thread_group_dispatch_params);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_node_data__thread_group_dispatch[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__thread_group_dispatch_t, thread_group, 1, PTYPE_thread_group),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_op_data__thread_group_dispatch_t, work, 1,
            archi_aggr_type__thread_group_work.top_level),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_op_data__thread_group_dispatch_t, callback, 1,
            archi_aggr_type__thread_group_callback.top_level),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_op_data__thread_group_dispatch_t, param, 1,
            archi_aggr_type__thread_group_dispatch_params.top_level),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_node_data__thread_group_dispatch = ARCHI_AGGR_TYPE(
        archi_dexgraph_op_data__thread_group_dispatch_t, MEMBERS_dexgraph_node_data__thread_group_dispatch);

