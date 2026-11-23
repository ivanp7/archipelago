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


/*****************************************************************************/
// Attributes format description
/*****************************************************************************/

/**
 * @enum
 * @brief Bit widths of attributes fields.
 *
 * Fields are listed from left to right (from most significant to least significant bits)
 * within each subgroup.
 */
enum {
    /**
     * @brief Width of the whole attributes type.
     */
    ARCHI_POINTER_ATTR_WIDTH = 64, // sizeof(archi_pointer_attr_t) * CHAR_BIT

    /**
     * @brief Width of the memory kind attribute.
     */
    ARCHI_POINTER_ATTR_WIDTH__MEM_KIND = 2,

    /**************************** Data attributes ****************************/

    /**
     * @brief Number of the bits distinguishing between primitive and complex data types.
     *
     * The bits meaning:
     * - all zeros: complex data type;
     * - other: primitive data type.
     */
    ARCHI_POINTER_ATTR_WIDTH__DATA_TYPE = 3,

    /*********************** Primitive data attributes ***********************/

    /**
     * @brief Width of the field defining split of the primitive data shape field.
     *
     * The number stored in the field defines how the shape field is split
     * into stride/alignment and length fields.
     *
     * @see ARCHI_POINTER_ATTR_WIDTH__PDATA_STRIDE_ALIGN
     * @see ARCHI_POINTER_ATTR_WIDTH__PDATA_LENGTH
     */
    ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE_SPLIT = ARCHI_POINTER_ATTR_WIDTH__DATA_TYPE + 3,

    /**
     * @brief Width of the primitive data shape field.
     *
     * This field is a combination of the variable-length stride/alignment and length fields.
     * Widths of the constituent fields is defined by the shape split field.
     */
    ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE = ARCHI_POINTER_ATTR_WIDTH
        - ARCHI_POINTER_ATTR_WIDTH__MEM_KIND
        - ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE_SPLIT,

    /**
     * @brief Width of the primitive data stride/alignment field.
     *
     * This field stores the value of stride divided by alignment minus one:
     * `(stride / alignment) - 1`.
     */
#define ARCHI_POINTER_ATTR_WIDTH__PDATA_STRIDE_ALIGN(shape_split)   \
    (((1 << ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE_SPLIT) - 1) - (shape_split))

    /**
     * @brief Width of the primitive data length field.
     *
     * This field stores the number of elements in data array.
     */
#define ARCHI_POINTER_ATTR_WIDTH__PDATA_LENGTH(shape_split)         \
    (ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE - ARCHI_POINTER_ATTR_WIDTH__PDATA_STRIDE_ALIGN((shape_split)))

    /************************ Complex data attributes ************************/

    /**
     * @brief Width of the data tag field.
     *
     * Data tags designate data types.
     * Different data tags correspond to different data types, except that
     * derived type tags are considered compatible to base type tags.
     * A derived tag is an extension of a base tag:
     * `<derived tag> = <extension part><separator><base tag>`.
     * The zero tag works as the wildcard compatible with any other data type (including primitive types).
     */
    ARCHI_POINTER_ATTR_WIDTH__DATA_TAG = ARCHI_POINTER_ATTR_WIDTH
        - ARCHI_POINTER_ATTR_WIDTH__MEM_KIND
        - ARCHI_POINTER_ATTR_WIDTH__DATA_TYPE,

    /**
     * @brief Width of separators in data tag field.
     *
     * A data tag separator is a sequence of the specified number of zero bits.
     */
    ARCHI_POINTER_ATTR_WIDTH__DATA_TAG_SEP = 4,

    /************************** Function attributes **************************/

    /**
     * @brief Width of the function tag field.
     *
     * Function tags designate function types.
     * Different function tags correspond to different function types.
     * The zero tag works as the wildcard compatible with any other function type.
     */
    ARCHI_POINTER_ATTR_WIDTH__FUNC_TAG = ARCHI_POINTER_ATTR_WIDTH
        - ARCHI_POINTER_ATTR_WIDTH__MEM_KIND,
};

/**
 * @enum
 * @brief Bit offsets of attributes fields.
 *
 * Fields are listed from left to right (from most significant to least significant bits)
 * within each subgroup.
 */
enum {
    /**
     * @brief Offset of the memory kind attribute.
     */
    ARCHI_POINTER_ATTR_OFFSET__MEM_KIND = ARCHI_POINTER_ATTR_WIDTH
        - ARCHI_POINTER_ATTR_WIDTH__MEM_KIND,

    /**************************** Data attributes ****************************/

    /**
     * @brief Offset of the bits distinguishing between primitive and complex data types.
     */
    ARCHI_POINTER_ATTR_OFFSET__DATA_TYPE = ARCHI_POINTER_ATTR_OFFSET__MEM_KIND
        - ARCHI_POINTER_ATTR_WIDTH__DATA_TYPE,

    /*********************** Primitive data attributes ***********************/

    /**
     * @brief Offset of the field defining split of the primitive data shape field.
     */
    ARCHI_POINTER_ATTR_OFFSET__PDATA_SHAPE_SPLIT = ARCHI_POINTER_ATTR_OFFSET__MEM_KIND
        - ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE_SPLIT,

    /**
     * @brief Offset of the primitive data stride/alignment field.
     */
#define ARCHI_POINTER_ATTR_OFFSET__PDATA_STRIDE_ALIGN(shape_split)  \
    (ARCHI_POINTER_ATTR_OFFSET__PDATA_SHAPE_SPLIT - ARCHI_POINTER_ATTR_WIDTH__PDATA_STRIDE_ALIGN((shape_split)))
};

/**
 * @def
 * @brief Mask for the memory kind attribute.
 */
#define ARCHI_POINTER_ATTR_MASK__MEM_KIND           \
    ((((archi_pointer_attr_t)1 << ARCHI_POINTER_ATTR_WIDTH__MEM_KIND) - 1)  \
     << ARCHI_POINTER_ATTR_OFFSET__MEM_KIND)

/*****************************************************************************/
// Attributes format limits
/*****************************************************************************/

/**
 * @def
 * @brief Maximum data size that can be represented by the attribute format.
 *
 * Size is stride times length: `stride * length`.
 */
#define ARCHI_POINTER_ATTR_MAX__DATA_SIZE           \
    (((size_t)1 << ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE) - 1)

/**
 * @def
 * @brief Maximum data stride that can be represented by the attribute format.
 */
#define ARCHI_POINTER_ATTR_MAX__DATA_STRIDE         \
    (((size_t)1 << (ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE - 1)))

/**
 * @def
 * @brief Maximum alignment that can be represented by the attribute format.
 */
#define ARCHI_POINTER_ATTR_MAX__DATA_ALIGNMENT      \
    ARCHI_POINTER_ATTR_MAX__DATA_STRIDE

/**
 * @def
 * @brief Maximum data tag that can be represented by the attribute format.
 */
#define ARCHI_POINTER_ATTR_MAX__DATA_TAG            \
    (((archi_pointer_attr_t)1 << ARCHI_POINTER_ATTR_WIDTH__DATA_TAG) - 1)

/**
 * @def
 * @brief Maximum function tag that can be represented by the attribute format.
 */
#define ARCHI_POINTER_ATTR_MAX__FUNC_TAG            \
    (((archi_pointer_attr_t)1 << ARCHI_POINTER_ATTR_WIDTH__FUNC_TAG) - 1)

/*****************************************************************************/
// Pointee memory kinds
/*****************************************************************************/

/**
 * @def
 * @brief Memory kind: generic data (persistent, readable memory).
 *
 * @note This memory is not guaranteed to be writable.
 */
#define ARCHI_POINTEE_MEM__DATA_GENERIC             \
    ((archi_pointer_attr_t)0x0 << ARCHI_POINTER_ATTR_OFFSET__MEM_KIND)

/**
 * @def
 * @brief Memory kind: data in persistent, writable memory.
 *
 * @note This memory is not guaranteed to be readable.
 */
#define ARCHI_POINTEE_MEM__DATA_WRITABLE            \
    ((archi_pointer_attr_t)0x1 << ARCHI_POINTER_ATTR_OFFSET__MEM_KIND)

/**
 * @def
 * @brief Memory kind: data in transient, readable memory.
 *
 * @note This memory is not guaranteed to be writable.
 */
#define ARCHI_POINTEE_MEM__DATA_TRANSIENT           \
    ((archi_pointer_attr_t)0x2 << ARCHI_POINTER_ATTR_OFFSET__MEM_KIND)

/**
 * @def
 * @brief Memory kind: function in executable memory.
 */
#define ARCHI_POINTEE_MEM__FUNCTION                 \
    ((archi_pointer_attr_t)0x3 << ARCHI_POINTER_ATTR_OFFSET__MEM_KIND)

/*****************************************************************************/
// Pointee types
/*****************************************************************************/

/**
 * @enum
 * @brief Types of pointees.
 */
enum {
    ARCHI_POINTEE_TYPE__UNSPEC = 0, ///< Type: unspecified/arbitrary/none.

    ARCHI_POINTEE_TYPE__PDATA,      ///< Type: primitive data.
    ARCHI_POINTEE_TYPE__CDATA,      ///< Type: complex data.
    ARCHI_POINTEE_TYPE__FUNC,       ///< Type: function.
};

/*****************************************************************************/
// Utility macros for generic pointers
/*****************************************************************************/

/**
 * @def
 * @brief Check if a pointer refers to a function.
 */
#define ARCHI_POINTER_TO_FUNCTION(attr)             \
    (((attr) & ARCHI_POINTER_ATTR_MASK__MEM_KIND)   \
     == ARCHI_POINTEE_MEM__FUNCTION)

/**
 * @def
 * @brief Check if a pointer refers to data.
 */
#define ARCHI_POINTER_TO_DATA(attr)                 \
    (!ARCHI_POINTER_TO_FUNCTION((attr)))

/**
 * @def
 * @brief Check if pointer refers to transient data.
 */
#define ARCHI_POINTER_TO_TRANSIENT_DATA(attr)       \
    (((attr) & ARCHI_POINTER_ATTR_MASK__MEM_KIND)   \
     == ARCHI_POINTEE_MEM__DATA_TRANSIENT)

/**
 * @def
 * @brief Check if a pointer refers to persistent data.
 */
#define ARCHI_POINTER_TO_PERSISTENT_DATA(attr)      \
    (ARCHI_POINTER_TO_DATA((attr))                  \
     && !ARCHI_POINTER_TO_TRANSIENT_DATA((attr)))

/**
 * @def
 * @brief Check if pointer refers to writable persistent data.
 */
#define ARCHI_POINTER_TO_WRITABLE_DATA(attr)        \
    (((attr) & ARCHI_POINTER_ATTR_MASK__MEM_KIND)   \
     == ARCHI_POINTEE_MEM__DATA_WRITABLE)

/*****************************************************************************/

/**
 * @def
 * @brief Check if pointer is null.
 */
#define ARCHI_POINTER_IS_NULL(pointer)              \
    ((ARCHI_POINTER_TO_FUNCTION((pointer).attr) ?   \
      (pointer).fptr == NULL : (pointer).ptr == NULL))

/**
 * @def
 * @brief Unset pointer address if it is primitive data of zero length.
 */
#define ARCHI_POINTER_NULLIFY_EMPTY(pointer) do {       \
    size_t archi_length;                                \
    if (archi_pointer_attr_unpk__pdata((pointer).attr,  \
                &archi_length, NULL, NULL, NULL) &&     \
            (archi_length == 0))                        \
        (pointer).ptr = NULL;                           \
} while (0)

/**
 * @def
 * @brief Initializer for a NULL pointer to function.
 *
 * This is to be used to initialize function pointers,
 * as `{0}` initializes data pointers instead.
 */
#define ARCHI_POINTER_NULL_FUNCTION                 \
    {.fptr = NULL, .attr = ARCHI_POINTEE_MEM__FUNCTION}

#endif // _ARCHI_BASE_POINTER_DEF_H_

