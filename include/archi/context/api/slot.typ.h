/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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
 * @brief Application context slot type.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_API_SLOT_TYP_H_
#define _ARCHI_CONTEXT_API_SLOT_TYP_H_

#include <stddef.h> // for ptrdiff_t, size_t

/**
 * @brief Context slot designator.
 */
typedef struct archi_context_slot {
    const char *name; ///< Name string.

    const ptrdiff_t *index; ///< Array of indices.
    size_t num_indices;  ///< Size of the array of indices.
} archi_context_slot_t;

#endif // _ARCHI_CONTEXT_API_SLOT_TYP_H_

