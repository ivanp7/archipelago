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
 * @brief Context interface for hashmaps.
 */

#pragma once
#ifndef _ARCHI_HASHMAP_CTX_HASHMAP_VAR_H_
#define _ARCHI_HASHMAP_CTX_HASHMAP_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: hashmap.
 *
 * Initialization parameters:
 * - "params"   : (archi_hashmap_alloc_params_t) hashmap creation parameters structure
 * - "capacity" : (size_t) hashmap internal array capacity
 *
 * Getter slots: any (without indices only) -- find a value associated with the key.
 *
 * Setter slots: any (without indices only) -- set a value associated with the key.
 * All setter slots are unsettable.
 */
extern
const archi_context_interface_t
archi_context_interface__hashmap;

#endif // _ARCHI_HASHMAP_CTX_HASHMAP_VAR_H_

