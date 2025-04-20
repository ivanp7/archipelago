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

#include "archi/util/size.fun.h"
#include "archi/util/size.def.h"

size_t
archi_size_array(
        archi_array_layout_t layout)
{
    if ((layout.num_of == 0) || (layout.size == 0) || (layout.alignment == 0))
        return 0;
    else if ((layout.alignment & (layout.alignment - 1)) != 0) // check if a power of two
        return 0;

    // Calculate element size including alignment padding
    size_t size_padded = ARCHI_SIZE_PADDED(layout.size, layout.alignment);

    // Calculate the full memory size (the last element doesn't need the alignment padding)
    size_t total_size = (layout.num_of - 1) * size_padded + layout.size;

    // Check if the full size doesn't in size_t
    if ((total_size - layout.size) / size_padded != layout.num_of - 1)
        return 0;

    return total_size;
}

