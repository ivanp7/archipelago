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
 * @brief Memory allocation utilities.
 */

#include "archi/util/alloc.fun.h"

#include <stdlib.h> // for malloc()
#include <string.h> // for strlen(), memcpy()

char*
archi_copy_string(
        const char *string)
{
    if (string == NULL)
        return NULL;

    size_t string_size = strlen(string) + 1;

    char *string_copy = malloc(string_size);
    if (string_copy == NULL)
        return NULL;

    memcpy(string_copy, string, string_size);

    return string_copy;
}

