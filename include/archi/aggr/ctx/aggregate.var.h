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
 * @brief Context interface for aggregate objects.
 */

#pragma once
#ifndef _ARCHI_AGGR_CTX_AGGREGATE_VAR_H_
#define _ARCHI_AGGR_CTX_AGGREGATE_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: aggregate object.
 *
 * Initialization parameters:
 * - "interface"    : (archi_aggr_interface_t) pointer to aggregate type interface
 * - "metadata"     : aggregate type metadata
 * - "fam_length"   : (size_t) flexible array length
 *
 * Getter slots:
 * - "interface"            : (archi_aggr_interface_t) pointer to aggregate type interface
 * - "metadata"             : aggregate type metadata
 * - "layout"               : (archi_layout_struct_t) aggregate type layout
 * - "size"                 : (size_t) aggregate object size
 * - "alignment"            : (size_t) aggregate object alignment requirement
 * - "fam_stride"           : (size_t) flexible array stride
 * - "fam_length"           : (size_t) flexible array length
 * - "full_size"            : (size_t) full size of aggregate object including flexible array member
 * - "object"               : aggregate object itself
 * - "member.***" <indices> : value of aggregate object member
 * - "ref.***" <indices>    : object referenced by aggregate object member
 *
 * Setter slots:
 * - "member.***" <indices> : aggregate object member
 */
extern
const archi_context_interface_t
archi_context_interface__aggregate;

#endif // _ARCHI_AGGR_CTX_AGGREGATE_VAR_H_

