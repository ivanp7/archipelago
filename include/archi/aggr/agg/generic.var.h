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
 * @brief Generic aggregate type interface for arbitrary types.
 */

#pragma once
#ifndef _ARCHI_AGGR_AGG_GENERIC_VAR_H_
#define _ARCHI_AGGR_AGG_GENERIC_VAR_H_

#include "archi/aggr/api/interface.typ.h"
#include "archi/aggr/agg/generic.typ.h"


/**
 * @brief Aggregate type interface: arbitrary type.
 *
 * This interface enables support for aggregate types described by
 * a special data structure listing all members and submembers of the type,
 * as well as all metadata required.
 *
 * Interface metadata type: archi_aggr_type_t.
 */
extern
const archi_aggr_interface_t
archi_aggr_interface__generic;

#endif // _ARCHI_AGGR_AGG_GENERIC_VAR_H_

