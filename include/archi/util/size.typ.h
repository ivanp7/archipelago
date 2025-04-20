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
 * @brief Types for storing sizes of objects.
 */

#pragma once
#ifndef _ARCHI_UTIL_SIZE_TYP_H_
#define _ARCHI_UTIL_SIZE_TYP_H_

#include <stddef.h> // for size_t

/**
 * @struct archi_array_layout_t
 * @brief Array layout description.
 *
 * This structure holds metadata about an array of elements in memory,
 * including the number of elements, the size of each element, and the
 * memory alignment requirement for each element.
 */
typedef struct archi_array_layout {
    size_t num_of;    ///< Number of elements.
    size_t size;      ///< Size of a data element.
    size_t alignment; ///< Data element alignment requirement.
} archi_array_layout_t;

#endif // _ARCHI_UTIL_SIZE_TYP_H_

