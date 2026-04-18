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

#pragma once
#ifndef _ARCHI_BASE_DPTR_ARRAY_FUN_H_
#define _ARCHI_BASE_DPTR_ARRAY_FUN_H_

#include "archi_base/dptr_array.typ.h"

#include <stdbool.h>


/**
 * @brief Allocate an array of data pointers.
 *
 * @note Returned pointer must be eventually released using free().
 *
 * @return Newly allocated array.
 */
archi_dptr_array_t
archi_dptr_array_alloc(
        size_t length ///< [in] Length of the array.
);

/**
 * @brief Change length of an array of data pointers.
 *
 * @return True on success, false on failure.
 */
bool
archi_dptr_array_set_length(
        archi_dptr_array_t *array, ///< [in,out] Data pointer array.
        size_t new_length ///< [in] New length of the array.
);

#endif // _ARCHI_BASE_DPTR_ARRAY_FUN_H_

