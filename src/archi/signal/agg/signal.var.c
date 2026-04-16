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
 * @brief Aggregate type descriptions for signal handlers.
 */

#include "archi/signal/agg/signal.var.h"
#include "archi/signal/exe/signal.typ.h"
#include "archi/signal/api/signal.typ.h"
#include "archi/signal/api/handler.typ.h"
#include "archi/signal/api/tag.def.h"


static
const archi_aggr_member_type__value_t
VTYPE_int = ARCHI_AGGR_MEMBER_TYPE__VALUE(int, 0);

static
const archi_aggr_member_type__value_t
VTYPE_size = ARCHI_AGGR_MEMBER_TYPE__VALUE(size_t, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_data = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(void*, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_signal_flags = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(archi_signal_flags_t*,
        ARCHI_POINTER_DATA_TAG__SIGNAL_FLAGS);

static
const archi_aggr_member_type__pointer_t
PTYPE_signal_handler_func = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_FUNC(archi_signal_handler_func_t,
        ARCHI_POINTER_FUNC_TAG__SIGNAL_HANDLER);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_signal_handler[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_signal_handler_t, function, 1, PTYPE_signal_handler_func),
    ARCHI_AGGR_MEMBER__POINTER(archi_signal_handler_t, data, 1, PTYPE_data),
};

const archi_aggr_type_t
archi_aggr_type__signal_handler = ARCHI_AGGR_TYPE(
        archi_signal_handler_t, 0,
        MEMBERS_signal_handler);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_transition_data__signal_detect__signal[] = {
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_transition_data__signal_detect__signal_t, index, 1, VTYPE_int),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_transition_data__signal_detect__signal_t, branch_index, 1, VTYPE_size),
};

static
const archi_aggr_member_type__aggregate_t
TYPE_dexgraph_transition_data__signal_detect__signal = ARCHI_AGGR_MEMBER_TYPE__AGGREGATE(
        archi_dexgraph_transition_data__signal_detect__signal_t, MEMBERS_dexgraph_transition_data__signal_detect__signal);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_transition_data__signal_detect[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_transition_data__signal_detect_t, flags, 1, PTYPE_signal_flags),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_transition_data__signal_detect_t, num_signals, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_transition_data__signal_detect_t, signal, 0,
            TYPE_dexgraph_transition_data__signal_detect__signal),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_transition_data__signal_detect = ARCHI_AGGR_TYPE(
        archi_dexgraph_transition_data__signal_detect_t, 0,
        MEMBERS_dexgraph_transition_data__signal_detect);

