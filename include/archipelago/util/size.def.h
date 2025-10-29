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
 * @brief Utilities related to sizes of objects.
 */

#pragma once
#ifndef _ARCHIPELAGO_UTIL_SIZE_DEF_H_
#define _ARCHIPELAGO_UTIL_SIZE_DEF_H_

/**
 * @def ARCHI_LENGTH_ARRAY(array)
 * @brief Get length of an array.
 *
 * @note This macro doesn't work with pointers.
 *
 * @return Number of elements in array.
 */
#define ARCHI_LENGTH_ARRAY(array)   \
    (sizeof((array)) / sizeof((array)[0]))

/**
 * @def ARCHI_SIZEOF_FLEXIBLE(type, member, length)
 * @brief Calculate the total size, in bytes, of a struct that contains a flexible array member.
 *
 * This macro is useful when you need to allocate a block of memory large enough
 * to hold a base struct plus a specified number of elements in its trailing
 * flexible array member.
 *
 * @param type     The name of the struct type that declares the flexible array member.
 * @param member   The name of the flexible array member within the struct.
 * @param length   The number of elements you wish to allocate in the flexible array.
 *
 * @return The total size in bytes.
 */
#define ARCHI_SIZEOF_FLEXIBLE(type, member, length) \
    (sizeof(type) + sizeof(((type*)NULL)->member[0]) * (length))

/**
 * @def ARCHI_SIZE_OVERFLOW(length, stride)
 * @brief Check if full size does not fit in the type.
 *
 * @param length   The number of data elements.
 * @param stride   Size of a data element.
 *
 * @note @p stride must be positive.
 *
 * @return True if the product overflows, otherwise false.
 */
#define ARCHI_SIZE_OVERFLOW(length, stride) \
    (((length) * (stride)) / (stride) != (length))

#endif // _ARCHIPELAGO_UTIL_SIZE_DEF_H_

