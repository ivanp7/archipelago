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
 * @brief Aggregate type descriptions for data of operation functions for printing formatted messages to streams.
 */

#include "archi/file/agg/print.var.h"
#include "archi/file/exe/print.typ.h"
#include "archi/file/exe/tag.def.h"
#include "archi/file/i/tag.def.h"


static
const archi_aggr_member_type__value_t
VTYPE_size = ARCHI_AGGR_MEMBER_TYPE__VALUE(size_t, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_string = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_PDATA(char*, char, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_pointer = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(archi_pointer_t, 0);

static
const archi_aggr_member_type__pointer_t
PTYPE_file = ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(FILE*,
        ARCHI_POINTER_DATA_TAG__FILE_STREAM);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_op_data__file_printf__token[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__file_printf__token_t, conv_spec, 1, PTYPE_string),
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__file_printf__token_t, argument, 1, PTYPE_pointer),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_op_data__file_printf__token = ARCHI_AGGR_TYPE(
        archi_dexgraph_op_data__file_printf__token_t, 0,
        MEMBERS_dexgraph_op_data__file_printf__token);

/*****************************************************************************/

static
const archi_aggr_member_t
MEMBERS_dexgraph_op_data__file_printf[] = {
    ARCHI_AGGR_MEMBER__POINTER(archi_dexgraph_op_data__file_printf_t, stream, 1, PTYPE_file),
    ARCHI_AGGR_MEMBER__VALUE(archi_dexgraph_op_data__file_printf_t, num_tokens, 1, VTYPE_size),
    ARCHI_AGGR_MEMBER__AGGREGATE(archi_dexgraph_op_data__file_printf_t, token, 0,
            archi_aggr_type__dexgraph_op_data__file_printf__token.top_level),
};

const archi_aggr_type_t
archi_aggr_type__dexgraph_op_data__file_printf = ARCHI_AGGR_TYPE_WITH_FAM(
        archi_dexgraph_op_data__file_printf_t,
        ARCHI_POINTER_DATA_TAG__DEXGRAPH_OP_DATA__FILE_PRINTF,
        archi_dexgraph_op_data__file_printf__token_t,
        MEMBERS_dexgraph_op_data__file_printf);

