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
 * @brief Operations on arrays of data pointers.
 */

#include "archi_base/util/dptr_array.fun.h"
#include "archi_base/util/size.def.h"

#include <stdlib.h> // for realloc()


archi_dptr_array_t
archi_dptr_array_alloc(
        size_t length)
{
    archi_dptr_array_t array = NULL;
    if (!archi_dptr_array_set_length(&array, length))
        return NULL;

    return array;
}

bool
archi_dptr_array_set_length(
        archi_dptr_array_t *array,
        size_t new_length)
{
    if (array == NULL)
        return false;

    size_t old_length = (*array != NULL) ? (*array)->length : 0;

    archi_dptr_array_t new_array = realloc(*array,
            ARCHI_SIZEOF_FLEXIBLE(struct archi_dptr_array, ptr, new_length));
    if (new_array == NULL)
        return false;

    {
        size_t *length_ptr = &new_array->length;
        *length_ptr = new_length;
    }
    for (size_t i = old_length; i < new_length; i++)
        new_array->ptr[i] = NULL;

    *array = new_array;
    return true;
}

