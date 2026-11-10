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
 * @brief The generic pointer type.
 */

#pragma once
#ifndef _ARCHI_BASE_POINTER_TYP_H_
#define _ARCHI_BASE_POINTER_TYP_H_

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
 * The pointer attributes format allows to represent 3 kinds of pointees:
 * 1. primitive data: identified by length, stride, alignment;
 * 2. complex data: identified by tag;
 * 3. function: identified by tag.
 *
 * Data pointer attributes for primitive data:
 * + bits (64; 62]   <2 bits>    -- memory type (equal to one of ARCHI_POINTER_TYPE__DATA_xxx constants);
 * + bits (62; 56]   <6 bits>    -- 63-N, where N is the bit width of the data stride/alignment attribute;
 * + bits (56; 56-N] <N bits>    -- (data stride) / (alignment requirement) - 1,
 *                                  with M leading zeros designating alignment (1 << M);
 * + bits (56-N; 0]  <56-N bits> -- data length;
 * where 0 <= M <= N < 56.
 *
 * Data pointer attributes for complex data:
 * + bits (64; 62]   <2 bits>    -- memory type (equal to one of ARCHI_POINTER_TYPE__DATA_xxx constants);
 * + bits (62; 59]   <3 bits>    -- zeroes: `000`;
 * + bits (59; 0]    <59 bits>   -- data tag.
 *
 * Function pointer attributes:
 * + bits (64; 62]   <2 bits>    -- equal to ARCHI_POINTER_TYPE__FUNCTION constant;
 * + bits (62; 0]    <62 bits>   -- function type tag.
 */
typedef uint64_t archi_pointer_attr_t;

/**
 * @struct archi_pointer_t
 * @brief Generic wrapper for data/function pointers with attributes.
 *
 * This structure can hold either:
 *   - `ptr` : a generic pointer to writable data;
 *   - `cptr`: a generic pointer to read-only data;
 *   - `fptr`: a generic pointer to a function.
 *
 * The used field depends on the pointer type stored in the attributes:
 *   - `ARCHI_POINTER_TYPE__DATA_ON_STACK` -- `ptr` is used;
 *   - `ARCHI_POINTER_TYPE__DATA_WRITABLE` -- `ptr` is used;
 *   - `ARCHI_POINTER_TYPE__DATA_READONLY` -- `cptr` is used;
 *   - `ARCHI_POINTER_TYPE__FUNCTION`      -- `fptr` is used.
 *
 * `attr` contains pointer attributes allowing to extract information about the pointee.
 * @see archi_pointer_attr_t
 */
typedef struct archi_pointer {
    union {
        archi_data_t ptr;      ///< Pointer to writable data.
        archi_rodata_t cptr;   ///< Pointer to read-only data.
        archi_function_t fptr; ///< Pointer to function.
    };
    archi_pointer_attr_t attr; ///< Pointer attributes.
} archi_pointer_t;

#endif // _ARCHI_BASE_POINTER_TYP_H_

