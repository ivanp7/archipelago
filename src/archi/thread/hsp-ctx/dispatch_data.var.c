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
 * @brief Context interface for data of thread group dispatch HSP state.
 */

#include "archi/thread/hsp-ctx/dispatch_data.var.h"
#include "archi/thread/hsp/dispatch.typ.h"
#include "archi/context/ctx/struct.fun.h"
#include "archipelago/util/struct.typ.h"
#include "archipelago/util/size.def.h"

#include <stdalign.h>

static
const archi_struct_field_info_t
vfields[] = {
    ARCHI_STRUCT_VFIELD_INFO(param.offset, archi_hsp_state_data__thread_group_dispatch_t,
            1, sizeof(size_t), alignof(size_t)),
    ARCHI_STRUCT_VFIELD_INFO(param.size, archi_hsp_state_data__thread_group_dispatch_t,
            1, sizeof(size_t), alignof(size_t)),
    ARCHI_STRUCT_VFIELD_INFO(param.batch_size, archi_hsp_state_data__thread_group_dispatch_t,
            1, sizeof(size_t), alignof(size_t)),
};

static
const archi_struct_field_info_t
pfields[] = {
    ARCHI_STRUCT_PFIELD_INFO__DATA(thread_group, archi_hsp_state_data__thread_group_dispatch_t,
            ARCHI_POINTER_DATA_TAG__THREAD_GROUP),

    ARCHI_STRUCT_PFIELD_INFO__FUNCTION(work.function, archi_hsp_state_data__thread_group_dispatch_t,
            ARCHI_POINTER_FUNCTION_TAG__THREAD_WORK),
    ARCHI_STRUCT_PFIELD_INFO__DATA(work.data, archi_hsp_state_data__thread_group_dispatch_t, 0),

    ARCHI_STRUCT_PFIELD_INFO__FUNCTION(callback.function, archi_hsp_state_data__thread_group_dispatch_t,
            ARCHI_POINTER_FUNCTION_TAG__THREAD_CALLBACK),
    ARCHI_STRUCT_PFIELD_INFO__DATA(callback.data, archi_hsp_state_data__thread_group_dispatch_t, 0),
};

static
const archi_hsp_state_data__thread_group_dispatch_t initializer = {0};

static
const archi_struct_info_t struct_info = {
    .vfield = vfields,
    .pfield = pfields,

    .num_vfields = ARCHI_LENGTH_ARRAY(vfields),
    .num_pfields = ARCHI_LENGTH_ARRAY(pfields),

    .size = sizeof(archi_hsp_state_data__thread_group_dispatch_t),
    .alignment = alignof(archi_hsp_state_data__thread_group_dispatch_t),

    .initializer = &initializer,
};

ARCHI_CONTEXT_INTERFACE_DEFINE_STRUCT(hsp_state_data__thread_group_dispatch, &struct_info);

