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
 * @brief Types for describing sizes of objects.
 */

#pragma once
#ifndef _ARCHI_BASE_UTIL_SIZE_TYP_H_
#define _ARCHI_BASE_UTIL_SIZE_TYP_H_

#include <stddef.h> // for size_t
#include <stdalign.h> // for alignof()


/**
 * @brief Layout of a type in memory.
 *
 * `alignment` must be a power of two.
 */
typedef struct archi_layout_type {
    size_t size;      ///< Size of a type in bytes.
    size_t alignment; ///< Alignment requirement of a type in bytes.
} archi_layout_type_t;

/**
 * @brief Literal of a type layout.
 */
#define ARCHI_LAYOUT_TYPE(type)  \
    {.size = sizeof(type), .alignment = alignof(type)}

/**
 * @brief Layout of a structure in memory.
 *
 * For structures without flexible array member, `fam_stride` is zero.
 * For types with flexible array member, `fam_stride` is non-zero.
 */
typedef struct archi_layout_struct {
    archi_layout_type_t base; ///< Layout of the structure without flexible array member.
    size_t fam_stride; ///< Stride of the flexible array member.
} archi_layout_struct_t;

/**
 * @brief Literal of a layout of a structure (without flexible array member).
 */
#define ARCHI_LAYOUT_STRUCT(type)   \
    {.base = ARCHI_LAYOUT_TYPE(type)}

/**
 * @brief Literal of a layout of a structure (with flexible array member).
 */
#define ARCHI_LAYOUT_STRUCT_WITH_FAM(type, fam) \
    {.base = ARCHI_LAYOUT_TYPE(type), .fam_stride = sizeof(((type*)NULL)->fam[0])}

/**
 * @brief Layout of an array in memory.
 */
typedef struct archi_layout_array {
    archi_layout_type_t base; ///< Layout of an array element.
    size_t length; ///< Length of array.
} archi_layout_array_t;

/**
 * @brief Literal of a layout of an array.
 */
#define ARCHI_LAYOUT_ARRAY(type, array_length)  \
    {.base = ARCHI_LAYOUT_TYPE(type), .length = (array_length)}

#endif // _ARCHI_BASE_UTIL_SIZE_TYP_H_

