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

#pragma once
#ifndef _ARCHI_UTIL_ALLOC_FUN_H_
#define _ARCHI_UTIL_ALLOC_FUN_H_

#include "archi/util/pointer.typ.h"
#include "archi/util/status.typ.h"

/**
 * @brief Resize array of values together with array of references to individual elements.
 *
 * This function is protected from intermediate memory allocation errors.
 * If such an error occurs, the original arrays are not modified.
 *
 * If the array is shrunk, reference counters of deleted elements are decremented.
 *
 * `new_size` can be zero, in which case arrays are freed and pointers set to NULL.
 * If `empty_element` is NULL, the new array memory is memset() to 0.
 *
 * @return Status code.
 */
archi_status_t
archi_resize_array(
        archi_pointer_t *array,     ///< [in,out] Pointer to array to resize.
        archi_pointer_t **elements, ///< [in,out] Array of pointers to individual elements.

        size_t new_num_elements, ///< [in] New array size.
        const void *new_element  ///< [in] Contents of new array elements.
);

#endif // _ARCHI_UTIL_ALLOC_FUN_H_

