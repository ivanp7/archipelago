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
 * @brief The data pointer array type.
 */

#pragma once
#ifndef _ARCHI_BASE_DPTR_ARRAY_TYP_H_
#define _ARCHI_BASE_DPTR_ARRAY_TYP_H_

#include <stddef.h> // for size_t


/**
 * @struct archi_dptr_array
 * @brief Array of pointers to data.
 */
struct archi_dptr_array {
    const size_t length; ///< Length of the array.
    void *ptr[]; ///< Array of data pointers.
};

/**
 * @typedef archi_dptr_array_t
 * @brief Pointer to array of pointers to data.
 */
typedef struct archi_dptr_array *archi_dptr_array_t;

#endif // _ARCHI_BASE_DPTR_ARRAY_TYP_H_

