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
 * @brief The pointer wrapper type.
 */

#pragma once
#ifndef _ARCHIPELAGO_BASE_POINTER_TYP_H_
#define _ARCHIPELAGO_BASE_POINTER_TYP_H_

#include "archipelago/base/ref_count.typ.h"

#include <stdint.h> // for uint64_t

/**
 * @typedef archi_data_t
 * @brief Generic writable data pointer type.
 */
typedef void *archi_data_t;

/**
 * @typedef archi_data_t
 * @brief Generic read-only data pointer type.
 */
typedef const void *archi_rodata_t;

/**
 * @typedef archi_function_t
 * @brief Generic function pointer type.
 *
 * This typedef defines a pointer to a function taking no parameters
 * and returning void. It can be used much like a `void*` for data:
 * any function type can be casted to `archi_function_t`,
 * then casted back to the correct signature before calling.
 *
 * @note `archi_function_t` must be casted back to the original
 *       function pointer type before invoking it to ensure the
 *       correct calling convention, arguments, and return type.
 *       Otherwise, the behavior is undefined.
 */
typedef void (*archi_function_t)(void);

/**
 * @typedef archi_pointer_attr_t
 * @brief 64-bit unsigned integer for storing pointer attributes.
 *
 * The pointer attributes format allows to represent 3 types of pointees:
 * 1. transparent data (data with known length, stride, and alignment);
 * 2. opaque data (data with unknown length, stride, and alignment) -- identified by tag;
 * 3. function -- identified by tag.
 *
 * Data pointer attributes:
 * + bits (64; 62]   <2 bits>    -- memory type (equal to one of ARCHI_POINTER_TYPE__DATA_xxx constants);
 * + bits (62; 56]   <6 bits>    -- bit width (denoted below as N) of the data stride/alignment attribute;
 * + bits (56; 56-N] <N bits>    -- (data stride) / (alignment requirement) - 1,
 *                                  with M leading zeros designating alignment (1 << M);
 * + bits (56-N; 0]  <56-N bits> -- data length;
 * where 0 <= M <= N <= 55.
 *
 * This scheme allows to describe data of size up to 64 PiB minus 1 byte,
 * with stride and alignment up to 32 PiB, which is enough for practical purposes.
 *
 * When N >= 56, meaning of bits (62; 0] changes: it is now bitwise negation of
 * a data type tag (integer in range [0; 0x07FFFFFFFFFFFFFF]) used to distinguish opaque data types from one another.
 * Tag #0 is special - it means 'unknown type' and is compatible with any other data type,
 * both transparent and opaque kinds.
 * Length, stride, and/or alignment of an object of an opaque type is unknown.
 *
 * Function pointer attributes:
 * + bits (64; 62]   <2 bits>    -- equal to ARCHI_POINTER_TYPE__FUNCTION constant;
 * + bits (62; 0]    <62 bits>   -- function type tag.
 *
 * Function type tags (integers in range [0; 0x3FFFFFFFFFFFFFFF]) are used to distinguish function types from one another.
 * Tag #0 is special - it means 'unknown type' and is compatible with any other function type.
 */
typedef uint64_t archi_pointer_attr_t;

/**
 * @struct archi_pointer_t
 * @brief Generic wrapper for data/function pointers with attributes.
 *
 * This structure can hold either:
 *   - `ptr` : a pointer to data or an opaque object;
 *   - `cptr`: a pointer to constant (read-only) data or an opaque object;
 *   - `fptr`: a generic function pointer to an arbitrary function.
 *
 * The used field depends on the pointer type stored in the attributes:
 *   - `ARCHI_POINTER_TYPE__DATA_ON_STACK` -- `ptr` is used;
 *   - `ARCHI_POINTER_TYPE__DATA_WRITABLE` -- `ptr` is used;
 *   - `ARCHI_POINTER_TYPE__DATA_READONLY` -- `cptr` is used;
 *   - `ARCHI_POINTER_TYPE__FUNCTION`      -- `fptr` is used.
 *
 * `attr` contains information about the pointer and memory behind it:
 * type of the pointer, type of the memory, data aligment & size.
 */
typedef struct archi_pointer {
    union {
        archi_data_t ptr;      ///< Pointer to writable data.
        archi_rodata_t cptr;   ///< Pointer to read-only data.
        archi_function_t fptr; ///< Pointer to function.
    };
    archi_pointer_attr_t attr; ///< Pointer attributes.
} archi_pointer_t;

/**
 * @struct archi_rcpointer_t
 * @brief Generic wrapper for reference counted data/function pointers with attributes.
 *
 * This type extends `archi_pointer_t` with a reference counter of the pointee.
 */
typedef struct archi_rcpointer {
    union {
        archi_pointer_t p; ///< Managed pointer.
        struct {
            union {
                archi_data_t ptr;      ///< Pointer to writable data.
                archi_rodata_t cptr;   ///< Pointer to read-only data.
                archi_function_t fptr; ///< Pointer to function.
            };
            archi_pointer_attr_t attr; ///< Pointer attributes.
        };
    };
    archi_reference_count_t ref_count; ///< Reference counter of the pointee.
} archi_rcpointer_t;

#endif // _ARCHIPELAGO_BASE_POINTER_TYP_H_

