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
 * @brief Macros and constants for the generic pointer type.
 */

#pragma once
#ifndef _ARCHI_BASE_POINTER_DEF_H_
#define _ARCHI_BASE_POINTER_DEF_H_

#include "archi_base/pointer.typ.h"

#include <limits.h> // for CHAR_BIT


/*****************************************************************************/
// Pointee kinds
/*****************************************************************************/

/**
 * @enum
 * @brief Kinds of pointees.
 */
enum {
    ARCHI_POINTEE__UNSPEC = 0,  ///< Unspecified/arbitrary/none.

    ARCHI_POINTEE__PDATA,   ///< Primitive data.
    ARCHI_POINTEE__CDATA,   ///< Complex data.
    ARCHI_POINTEE__FUNC,    ///< Function.
};

/*****************************************************************************/
// Limits
/*****************************************************************************/

/**
 * @def ARCHI_POINTER_DATA_SIZE_MAX
 * @brief Maximum data size that can be represented by the attribute format.
 */
#define ARCHI_POINTER_DATA_SIZE_MAX                 \
    (((size_t)1 << ARCHI_POINTER_ATTR_BITWIDTH__PDATA_SHAPE) - 1)

/**
 * @def ARCHI_POINTER_DATA_STRIDE_MAX
 * @brief Maximum data stride that can be represented by the attribute format.
 */
#define ARCHI_POINTER_DATA_STRIDE_MAX               \
    (((size_t)1 << (ARCHI_POINTER_ATTR_BITWIDTH__PDATA_SHAPE - 1)))

/**
 * @def ARCHI_POINTER_DATA_ALIGNMENT_MAX
 * @brief Maximum alignment that can be represented by the attribute format.
 */
#define ARCHI_POINTER_DATA_ALIGNMENT_MAX            \
    ARCHI_POINTER_DATA_STRIDE_MAX

/**
 * @def ARCHI_POINTER_DATA_TAG_MAX
 * @brief Maximum data tag that can be represented by the attribute format.
 */
#define ARCHI_POINTER_DATA_TAG_MAX                  \
    (((archi_pointer_attr_t)1 << ARCHI_POINTER_ATTR_BITWIDTH__DATA_TAG) - 1)

/**
 * @def ARCHI_POINTER_FUNC_TAG_MAX
 * @brief Maximum function tag that can be represented by the attribute format.
 */
#define ARCHI_POINTER_FUNC_TAG_MAX                  \
    (((archi_pointer_attr_t)1 << ARCHI_POINTER_ATTR_BITWIDTH__FUNC_TAG) - 1)

/*****************************************************************************/
// Bit widths of attributes
/*****************************************************************************/

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH
 * @brief Bit width of archi_pointer_attr_t (total number of bits).
 */
#define ARCHI_POINTER_ATTR_BITWIDTH                 \
    (sizeof(archi_pointer_attr_t) * CHAR_BIT)

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__TYPE
 * @brief Bit width of the memory type attribute.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__TYPE           2

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__DATA_TYPE
 * @brief Bit width of the data type attribute.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__DATA_TYPE      3

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__PDATA_TYPE_EXT
 * @brief Bit width of primitive data type attribute extention.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__PDATA_TYPE_EXT 3

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__PDATA_SWIDTH
 * @brief Bit width of primitive data type attribute (shape split).
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__PDATA_SWIDTH   \
    (ARCHI_POINTER_ATTR_BITWIDTH__DATA_TYPE         \
     + ARCHI_POINTER_ATTR_BITWIDTH__PDATA_TYPE_EXT)

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__PDATA_SHAPE
 * @brief Total bit width of data length and stride/alignment attributes.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__PDATA_SHAPE    \
    (ARCHI_POINTER_ATTR_BITWIDTH                    \
     - ARCHI_POINTER_ATTR_BITWIDTH__TYPE            \
     - ARCHI_POINTER_ATTR_BITWIDTH__PDATA_SWIDTH)

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__PDATA_LENGTH
 * @brief Bit width of data length attribute.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__PDATA_LENGTH(N)\
    (ARCHI_POINTER_ATTR_BITWIDTH__PDATA_SHAPE - (N))

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__DATA_TAG
 * @brief Bit width of a data tag.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__DATA_TAG       \
    (ARCHI_POINTER_ATTR_BITWIDTH                    \
     - ARCHI_POINTER_ATTR_BITWIDTH__TYPE            \
     - ARCHI_POINTER_ATTR_BITWIDTH__DATA_TYPE)

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__DATA_TAG_SEP
 * @brief Bit width of the token separator in data tags.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__DATA_TAG_SEP   4

/**
 * @def ARCHI_POINTER_ATTR_BITWIDTH__FUNC_TAG
 * @brief Bit width of a function tag.
 */
#define ARCHI_POINTER_ATTR_BITWIDTH__FUNC_TAG       \
    (ARCHI_POINTER_ATTR_BITWIDTH                    \
     - ARCHI_POINTER_ATTR_BITWIDTH__TYPE)

/*****************************************************************************/
// Offsets of attributes
/*****************************************************************************/

/**
 * @def ARCHI_POINTER_ATTR_OFFSET__TYPE
 * @brief Offset to memory type attribute.
 */
#define ARCHI_POINTER_ATTR_OFFSET__TYPE             \
    (ARCHI_POINTER_ATTR_BITWIDTH                    \
     - ARCHI_POINTER_ATTR_BITWIDTH__TYPE)

/**
 * @def ARCHI_POINTER_ATTR_OFFSET__DATA_TYPE
 * @brief Offset to the data type attribute.
 */
#define ARCHI_POINTER_ATTR_OFFSET__DATA_TYPE        \
    (ARCHI_POINTER_ATTR_OFFSET__TYPE                \
     - ARCHI_POINTER_ATTR_BITWIDTH__DATA_TYPE)

/**
 * @def ARCHI_POINTER_ATTR_OFFSET__PDATA_SWIDTH
 * @brief Offset to primitive data type attribute (shape split).
 */
#define ARCHI_POINTER_ATTR_OFFSET__PDATA_SWIDTH     \
    (ARCHI_POINTER_ATTR_OFFSET__DATA_TYPE           \
     - ARCHI_POINTER_ATTR_BITWIDTH__PDATA_TYPE_EXT)

/**
 * @def ARCHI_POINTER_ATTR_OFFSET__PDATA_SOA
 * @brief Offset to 'stride over (divided by) alignment' attribute.
 */
#define ARCHI_POINTER_ATTR_OFFSET__PDATA_SOA(N)     \
    ARCHI_POINTER_ATTR_BITWIDTH__PDATA_LENGTH((N))

/*****************************************************************************/
// Masks of attributes
/*****************************************************************************/

/**
 * @def ARCHI_POINTER_ATTR_MASK__TYPE
 * @brief Attribute mask for memory type.
 */
#define ARCHI_POINTER_ATTR_MASK__TYPE               \
    ((((archi_pointer_attr_t)1 << ARCHI_POINTER_ATTR_BITWIDTH__TYPE) - 1)    \
     << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/*****************************************************************************/
// Memory type attribute
/*****************************************************************************/

/**
 * @def ARCHI_POINTER_TYPE__DATA_ON_STACK
 * @brief Pointer type: short-lived data, writable memory.
 */
#define ARCHI_POINTER_TYPE__DATA_ON_STACK           \
    ((archi_pointer_attr_t)0x0 << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/**
 * @def ARCHI_POINTER_TYPE__DATA_WRITABLE
 * @brief Pointer type: long-lived data, writable memory.
 */
#define ARCHI_POINTER_TYPE__DATA_WRITABLE           \
    ((archi_pointer_attr_t)0x1 << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/**
 * @def ARCHI_POINTER_TYPE__DATA_READONLY
 * @brief Pointer type: long-lived data, read-only memory.
 */
#define ARCHI_POINTER_TYPE__DATA_READONLY           \
    ((archi_pointer_attr_t)0x2 << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/**
 * @def ARCHI_POINTER_TYPE__FUNCTION
 * @brief Pointer type: function.
 */
#define ARCHI_POINTER_TYPE__FUNCTION                \
    ((archi_pointer_attr_t)0x3 << ARCHI_POINTER_ATTR_OFFSET__TYPE)

/*****************************************************************************/
// Utility macros
/*****************************************************************************/

/**
 * @brief Check if pointer refers to stack memory.
 */
#define ARCHI_POINTER_TO_STACK(attr)                \
    (((attr) & ARCHI_POINTER_ATTR_MASK__TYPE)       \
     == ARCHI_POINTER_TYPE__DATA_ON_STACK)

/**
 * @brief Check if pointer refers to writable memory (excluding stack memory).
 */
#define ARCHI_POINTER_TO_WRITABLE_DATA(attr)        \
    (((attr) & ARCHI_POINTER_ATTR_MASK__TYPE)       \
     == ARCHI_POINTER_TYPE__DATA_WRITABLE)

/**
 * @brief Check if pointer refers to read-only memory.
 */
#define ARCHI_POINTER_TO_READONLY_DATA(attr)        \
    (((attr) & ARCHI_POINTER_ATTR_MASK__TYPE)       \
     == ARCHI_POINTER_TYPE__DATA_READONLY)

/**
 * @brief Check if a pointer refers to a function.
 */
#define ARCHI_POINTER_TO_FUNCTION(attr)             \
    (((attr) & ARCHI_POINTER_ATTR_MASK__TYPE)       \
     == ARCHI_POINTER_TYPE__FUNCTION)

/**
 * @brief Check if pointer is null.
 */
#define ARCHI_POINTER_IS_NULL(pointer)              \
    ((ARCHI_POINTER_TO_FUNCTION((pointer).attr) ?   \
      (pointer).fptr == NULL : (pointer).ptr == NULL))

/**
 * @brief Unset pointer address if it is primitive data of zero length.
 */
#define ARCHI_POINTER_NULLIFY_EMPTY(pointer) do {       \
    size_t archi_length;                                \
    if (archi_pointer_attr_unpk__pdata((pointer).attr,  \
                &archi_length, NULL, NULL, NULL) &&     \
            (archi_length == 0))                        \
        (pointer).ptr = NULL;                           \
} while (0)

#endif // _ARCHI_BASE_POINTER_DEF_H_

