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
 * @brief Key-value list type with reference counted values.
 */

#pragma once
#ifndef _ARCHI_BASE_KRCVLIST_TYP_H_
#define _ARCHI_BASE_KRCVLIST_TYP_H_

#include "archi_base/kvlist.typ.h"
#include "archi_base/rcpointer.typ.h"


/**
 * @brief List of key-value pairs (with reference counted values).
 */
typedef struct archi_krcvlist {
    archi_kvlist_base_t n; ///< Node base.
    archi_rcpointer_t value; ///< Value.
} archi_krcvlist_t;

#endif // _ARCHI_BASE_KRCVLIST_TYP_H_

