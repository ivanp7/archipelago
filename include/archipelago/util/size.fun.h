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
 * @brief Functions for calculating sizes of objects.
 */

#pragma once
#ifndef _ARCHIPELAGO_UTIL_SIZE_FUN_H_
#define _ARCHIPELAGO_UTIL_SIZE_FUN_H_

#include "archipelago/base/size.typ.h"

/**
 * @brief Calculate the size of memory needed to hold
 * the contents of an array with the specified layout.
 *
 * @param [in] layout
 *   Array layout description. Number of data elements must not be 0.
 *   Size of a data element in bytes must not be 0.
 *   Alignment requirement a data element in bytes must be a power of two.
 *
 * @return Number of bytes needed to hold array contents, or 0 if parameters are invalid.
 */
size_t
archi_size_array(
        archi_array_layout_t layout
);

#endif // _ARCHIPELAGO_UTIL_SIZE_FUN_H_

