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
 * @brief Macros for calculating sizes of objects.
 */

#pragma once
#ifndef _ARCHI_UTIL_SIZE_DEF_H_
#define _ARCHI_UTIL_SIZE_DEF_H_

/**
 * @def ARCHI_SIZEOF_FLEXIBLE(type, member, num_of)
 * @brief Calculate the total size, in bytes, of a struct that contains a flexible array member.
 *
 * This macro is useful when you need to allocate a block of memory large enough
 * to hold a base struct plus a specified number of elements in its trailing
 * flexible array member.
 *
 * @param type     The name of the struct type that declares the flexible array member.
 * @param member   The name of the flexible array member within the struct.
 * @param num_of   The number of elements you wish to allocate in the flexible array.
 *
 * @return The total size in bytes.
 */
#define ARCHI_SIZEOF_FLEXIBLE(type, member, num_of) \
    (sizeof(type) + sizeof(((type*)NULL)->member[0]) * (num_of))

/**
 * @def ARCHI_SIZE_PADDED(size, alignment)
 * @brief Calculate the total size, in bytes, of an object including padding for alignment.
 *
 * This macro rounds up the given @p size to the nearest multiple of @p alignment,
 * ensuring the element size respects the specified memory alignment requirements.
 *
 * @param size      The original size of the element in bytes.
 * @param alignment The alignment requirement in bytes (must be a power of two).
 *
 * @return The size of the element rounded up to the nearest multiple of @p alignment.
 *
 * @note This macro assumes @p alignment is a power of two.
 */
#define ARCHI_SIZE_PADDED(size, alignment) \
    (((size) + ((alignment) - 1)) & ~((alignment) - 1))

#endif // _ARCHI_UTIL_SIZE_DEF_H_

