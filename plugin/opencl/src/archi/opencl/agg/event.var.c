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
 * @brief Aggregate type descriptions for data of DEG operation functions for OpenCL event operations.
 */

#include "archi/opencl/agg/event.var.h"
#include "archi/opencl/exe/event.typ.h"
#include "archi/opencl/api/event.typ.h"
#include "archi/opencl/api/tag.def.h"
#include "archi/timer/api/tag.def.h"


static
const archi_aggr_member_type__value_t
VTYPE_size = ARCHI_AGGR_MEMBER_TYPE__VALUE(size_t, 0);

static
const archi_aggr_member_type__value_t
VTYPE_uint = ARCHI_AGGR_MEMBER_TYPE__VALUE(cl_uint, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_timer = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(archi_timer_t,
        ARCHI_POINTER_DATA_TAG__TIMER);

static
const archi_aggr_member_type__pointer_t
PTYPE_event = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(cl_event,
        ARCHI_POINTER_DATA_TAG__OPENCL_EVENT);

static
const archi_aggr_member_type__pointer_t
PTYPE_event_ptr = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_PDATA(cl_event*, cl_event, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_event_ptr_ptr = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_PDATA(cl_event**, cl_event*, 0);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_opencl_event_array[] = {
    ARCHI_AGGR_MEMBER__VALUE(archi_opencl_event_array_t, num_events, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__POINTER(archi_opencl_event_array_t, event, 1, PTYPE_event_ptr),
};

const archi_aggr_type_t
archi_aggr_type__opencl_event_array = ARCHI_AGGR_TYPE(
        archi_opencl_event_array_t, 0,
        MEMBERS_opencl_event_array);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_opencl_event_ptr_array[] = {
    ARCHI_AGGR_MEMBER__VALUE(archi_opencl_event_ptr_array_t, num_event_ptrs, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__POINTER(archi_opencl_event_ptr_array_t, event_ptr, 1, PTYPE_event_ptr_ptr),
};

const archi_aggr_type_t
archi_aggr_type__opencl_event_ptr_array = ARCHI_AGGR_TYPE(
        archi_opencl_event_ptr_array_t, 0,
        MEMBERS_opencl_event_ptr_array);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_op_data__opencl_event_profile[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__opencl_event_profile_t, event, 1, PTYPE_event),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__opencl_event_profile_t, timer, 1, PTYPE_timer),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_op_data__opencl_event_profile_t, from_time, 1, VTYPE_uint),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_op_data__opencl_event_profile_t, to_time, 1, VTYPE_uint),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_op_data__opencl_event_profile = ARCHI_AGGR_TYPE(
        archi_dexgraph_op_data__opencl_event_profile_t, 0,
        MEMBERS_dexgraph_op_data__opencl_event_profile);

