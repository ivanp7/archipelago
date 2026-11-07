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
 * @brief Key-value list types.
 */

#pragma once
#ifndef _ARCHI_BASE_KVLIST_TYP_H_
#define _ARCHI_BASE_KVLIST_TYP_H_

#include "archi_base/pointer.typ.h"


struct archi_kvlist_base;

/**
 * @brief List of key-value pairs (base).
 */
typedef struct archi_kvlist_base {
    const struct archi_kvlist_base *next; ///< Pointer to the next list node.
    const char *key; ///< Key.
} archi_kvlist_base_t;

/**
 * @brief List of key-value pairs.
 */
typedef struct archi_kvlist {
    archi_kvlist_base_t n; ///< Node base.
    archi_pointer_t value; ///< Value.
} archi_kvlist_t;

#endif // _ARCHI_BASE_KVLIST_TYP_H_

