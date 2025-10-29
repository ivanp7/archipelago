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
 * @brief Macros and constants for pointer wrapper types.
 */

#pragma once
#ifndef _ARCHIPELAGO_BASE_POINTER_DEF_H_
#define _ARCHIPELAGO_BASE_POINTER_DEF_H_

#include "archipelago/base/pointer.typ.h"

#include <limits.h> // for CHAR_BIT

/*****************************************************************************/
// Pointer types
/*****************************************************************************/

/**
 * @def ARCHI_POINTER_TYPE_MASK
 * @brief Attribute mask for pointer type.
 */
#define ARCHI_POINTER_TYPE_MASK                     \
    ((archi_pointer_attr_t)0x3 << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/**
 * @def ARCHI_POINTER_TYPE__DATA_ON_STACK
 * @brief Pointer type: short-lived data, writable memory.
 */
#define ARCHI_POINTER_TYPE__DATA_ON_STACK   /*00*/  \
    ((archi_pointer_attr_t)0x0 << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/**
 * @def ARCHI_POINTER_TYPE__DATA_WRITABLE
 * @brief Pointer type: long-lived data, writable memory.
 */
#define ARCHI_POINTER_TYPE__DATA_WRITABLE   /*01*/  \
    ((archi_pointer_attr_t)0x1 << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/**
 * @def ARCHI_POINTER_TYPE__DATA_READONLY
 * @brief Pointer type: long-lived data, read-only memory.
 */
#define ARCHI_POINTER_TYPE__DATA_READONLY   /*10*/  \
    ((archi_pointer_attr_t)0x2 << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/**
 * @def ARCHI_POINTER_TYPE__FUNCTION
 * @brief Pointer type: function.
 */
#define ARCHI_POINTER_TYPE__FUNCTION        /*11*/  \
    ((archi_pointer_attr_t)0x3 << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/*****************************************************************************/
// Limits
/*****************************************************************************/

/**
 * @def ARCHI_POINTER_DATA_SIZE_MAX
 * @brief Maximum data size that can be stored in pointer attributes.
 */
#define ARCHI_POINTER_DATA_SIZE_MAX                 \
    (((size_t)1 << ARCHI_POINTER_ATTR_BITWIDTH__SIZE) - 1)

/**
 * @def ARCHI_POINTER_DATA_STRIDE_MAX
 * @brief Maximum data stride that can be stored in pointer attributes.
 */
#define ARCHI_POINTER_DATA_STRIDE_MAX               \
    (((size_t)1 << (ARCHI_POINTER_ATTR_BITWIDTH__SIZE - 1)))

/**
 * @def ARCHI_POINTER_DATA_ALIGNMENT_MAX
 * @brief Maximum alignment that can be stored in pointer attributes.
 */
#define ARCHI_POINTER_DATA_ALIGNMENT_MAX            \
    ARCHI_POINTER_DATA_STRIDE_MAX

/**
 * @def ARCHI_POINTER_DATA_TAG_MAX
 * @brief Maximum tag for an opaque data type.
 */
#define ARCHI_POINTER_DATA_TAG_MAX                  \
    ((~((archi_pointer_attr_t)ARCHI_POINTER_ATTR_BITWIDTH__SIZE \
       << ARCHI_POINTER_ATTR_OFFSET__STRIDE_WIDTH)) &           \
       (((archi_pointer_attr_t)1 << ARCHI_POINTER_ATTR_OFFSET__TYPE) - 1))

/**
 * @def ARCHI_POINTER_FUNCTION_TAG_MAX
 * @brief Maximum tag for a function type.
 */
#define ARCHI_POINTER_FUNCTION_TAG_MAX              \
    (((archi_pointer_attr_t)1 << ARCHI_POINTER_ATTR_OFFSET__TYPE) - 1)

/*****************************************************************************/
// Bit widths of attributes
/*****************************************************************************/

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH
 * @brief Bit width of archi_pointer_attr_t type (total number of bits).
 */
#define ARCHI_POINTER_ATTR_BITWIDTH                 \
    (sizeof(archi_pointer_attr_t) * CHAR_BIT)

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__TYPE
 * @brief Bit width of pointer type attribute.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__TYPE           2

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__STRIDE_WIDTH
 * @brief Bit width of stride bit size attribute.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__STRIDE_WIDTH   6

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__SIZE
 * @brief Total bit width of data length and stride/alignment attributes.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__SIZE           \
    (ARCHI_POINTER_ATTR_BITWIDTH                    \
     - ARCHI_POINTER_ATTR_BITWIDTH__TYPE            \
     - ARCHI_POINTER_ATTR_BITWIDTH__STRIDE_WIDTH)

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__LENGTH
 * @brief Bit width of data length attribute.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__LENGTH(stride_width)   \
    (ARCHI_POINTER_ATTR_BITWIDTH__SIZE - stride_width)

/*****************************************************************************/
// Offsets of attributes
/*****************************************************************************/

/**
 * @def ARCHI_POINTER_ATTR_OFFSET__TYPE
 * @brief Offset to pointer type attribute.
 */
#define ARCHI_POINTER_ATTR_OFFSET__TYPE             \
    (ARCHI_POINTER_ATTR_BITWIDTH                    \
     - ARCHI_POINTER_ATTR_BITWIDTH__TYPE)

/**
 * @def ARCHI_POINTER_ATTR_OFFSET__STRIDE_WIDTH
 * @brief Offset to stride bit size attribute.
 */
#define ARCHI_POINTER_ATTR_OFFSET__STRIDE_WIDTH     \
    (ARCHI_POINTER_ATTR_OFFSET__TYPE                \
     - ARCHI_POINTER_ATTR_BITWIDTH__STRIDE_WIDTH)

/**
 * @def ARCHI_POINTER_ATTR_OFFSET__STRIDE_OVER_ALIGNMENT
 * @brief Offset to stride/alignment attribute.
 */
#define ARCHI_POINTER_ATTR_OFFSET__STRIDE_OVER_ALIGNMENT(stride_width)  \
    ARCHI_POINTER_ATTR_BITWIDTH__LENGTH((stride_width))

/*****************************************************************************/
// Utility macros
/*****************************************************************************/

/**
 * @brief Check if pointer refers to stack memory.
 */
#define ARCHI_POINTER_TO_STACK(attr)                \
    (((attr) & ARCHI_POINTER_TYPE_MASK) == ARCHI_POINTER_TYPE__DATA_ON_STACK)

/**
 * @brief Check if pointer refers to writable memory (excluding stack memory).
 */
#define ARCHI_POINTER_TO_WRITABLE_DATA(attr)        \
    (((attr) & ARCHI_POINTER_TYPE_MASK) == ARCHI_POINTER_TYPE__DATA_WRITABLE)

/**
 * @brief Check if pointer refers to read-only memory.
 */
#define ARCHI_POINTER_TO_READONLY_DATA(attr)        \
    (((attr) & ARCHI_POINTER_TYPE_MASK) == ARCHI_POINTER_TYPE__DATA_READONLY)

/**
 * @brief Check if a pointer refers to a function.
 */
#define ARCHI_POINTER_TO_FUNCTION(attr)             \
    (((attr) & ARCHI_POINTER_TYPE_MASK) == ARCHI_POINTER_TYPE__FUNCTION)

/**
 * @brief Check if pointer is null.
 */
#define ARCHI_POINTER_IS_NULL(pointer)              \
    ((ARCHI_POINTER_TO_FUNCTION((pointer).attr) ? (pointer).fptr == NULL : (pointer).ptr == NULL))

#endif // _ARCHIPELAGO_BASE_POINTER_DEF_H_

