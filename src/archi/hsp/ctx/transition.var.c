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
 * @brief Context interface for hierarchical state processor transitions.
 */

#include "archi/hsp/ctx/transition.var.h"
#include "archi/hsp/api/transition.typ.h"
#include "archi/context/ctx/struct.fun.h"
#include "archipelago/util/struct.typ.h"
#include "archipelago/util/size.def.h"

#include <stdalign.h>

static
const archi_struct_field_info_t
pfields[] = {
    ARCHI_STRUCT_PFIELD_INFO__FUNCTION(function, archi_hsp_transition_t,
            ARCHI_POINTER_FUNCTION_TAG__HSP_TRANSITION),
    ARCHI_STRUCT_PFIELD_INFO__DATA(data, archi_hsp_transition_t, 0),
};

static
const archi_hsp_transition_t
initializer = {0};

static
const archi_struct_info_t
struct_info = {
    .pfield = pfields,

    .num_pfields = ARCHI_LENGTH_ARRAY(pfields),

    .size = sizeof(archi_hsp_transition_t),
    .alignment = alignof(archi_hsp_transition_t),

    .initializer = &initializer,
};

ARCHI_CONTEXT_INTERFACE_DEFINE_STRUCT(hsp_transition, &struct_info);

