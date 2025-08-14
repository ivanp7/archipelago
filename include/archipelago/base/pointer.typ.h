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
 * @brief The pointer wrapper type.
 */

#pragma once
#ifndef _ARCHIPELAGO_BASE_POINTER_TYP_H_
#define _ARCHIPELAGO_BASE_POINTER_TYP_H_

#include "archipelago/base/ref_count.fun.h"
#include "archipelago/base/size.typ.h"

#include <stdint.h> // for uint64_t
#include <limits.h> // for CHAR_BIT

/**
 * @typedef archi_function_t
 * @brief Generic placeholder for any function pointer.
 *
 * This typedef defines a pointer to a function taking no parameters
 * and returning void. It can be used much like a `void *` for data:
 * you store any function’s address in an `archi_function_t`, then
 * cast it back to the correct signature before calling.
 *
 * @note You must cast an `archi_function_t` back to the original
 *       function pointer type before invoking it to ensure the
 *       correct calling convention, arguments, and return type.
 *       Otherwise, the behavior is undefined.
 */
typedef void (*archi_function_t)(void);

/**
 * @typedef archi_pointer_flags_t
 * @brief Flags describing pointer attributes.
 */
typedef uint64_t archi_pointer_flags_t;

/**
 * @struct archi_pointer_t
 * @brief Generic wrapper for data or function pointers with metadata.
 *
 * This structure can hold either:
 *   - `ptr` : a pointer to a resource or (array of) data;
 *   - `fptr`: a generic function pointer to one or more functions.
 *
 * The flag ARCHI_POINTER_FLAG_FUNCTION in `flags` allows
 * to determine which union field is used.
 * Other bits are available for user.
 *
 * If pointer to function is stored, `element` field has no specified meaning.
 *
 * `element.num_of` stores the number of elements in the data array.
 * It must not be 0 unless the pointer is NULL, otherwise it must be 0.
 *
 * `element.size` stores the size in bytes of a data array element,
 * or 0 if it is unspecified or unknown.
 *
 * `element.alignment` stores the alignment requirement in bytes of a data array element,
 * or 0 if it is unspecified or unknown. If it is not 0, it must be a power of two.
 *
 * `ref_count` may point to the reference counter associated with the resource.
 * Resource consumers must increment the counter if they rely on the value,
 * and decrement it when they no longer need it.
 */
typedef struct archi_pointer {
    union {
        void *ptr;             ///< Pointer to a resource (a single object or array).
        archi_function_t fptr; ///< Generic function pointer (to single function or array).
    };
    archi_reference_count_t ref_count; ///< Reference count for the resource.
    archi_pointer_flags_t flags; ///< Flags describing pointer attributes.
    archi_array_layout_t element; ///< Layout of the data array.
} archi_pointer_t;

/**
 * @def ARCHI_POINTER_FLAG_FUNCTION
 * @brief Use the function pointer union field (`fptr`)
 *        instead of data pointer union field (`ptr`).
 *
 * Indicates that the wrapped pointer refers to function(s), not data.
 *
 * @note This flag is mutually exclusive with ARCHI_POINTER_FLAG_WRITABLE,
 * as code section is practically never writable.
 *
 * @note This flag is stored in the highest-order bit.
 */
#define ARCHI_POINTER_FLAG_FUNCTION         \
    ((archi_pointer_flags_t)1 << (sizeof(archi_pointer_flags_t) * CHAR_BIT - 1))

/**
 * @def ARCHI_POINTER_FLAG_WRITABLE
 * @brief Writing to the memory pointed by union field `ptr` is allowed.
 *
 * Indicates that the wrapped pointer refers to writable memory.
 *
 * @warning If this flag is not set, writing to the memory referred by `ptr`
 * invokes undefined behavior.
 *
 * @note This flag is mutually exclusive with ARCHI_POINTER_FLAG_FUNCTION,
 * as code section is practically never writable.
 */
#define ARCHI_POINTER_FLAG_WRITABLE         \
    (ARCHI_POINTER_FLAG_FUNCTION >> 1)

/**
 * @def ARCHI_POINTER_BUILTIN_FLAGS_BITS
 * @brief Number of bits used by the built-in flags.
 */
#define ARCHI_POINTER_BUILTIN_FLAGS_BITS    2

/**
 * @def ARCHI_POINTER_USER_FLAGS_BITS
 * @brief Number of bits available for user-defined flags.
 *
 * This specifies how many low-order bits can be used by the user without overlapping
 * the built-in flags stored in ARCHI_POINTER_BUILTIN_FLAGS_BITS high-order bits.
 */
#define ARCHI_POINTER_USER_FLAGS_BITS       \
    (sizeof(((archi_pointer_t*)NULL)->flags) * CHAR_BIT - ARCHI_POINTER_BUILTIN_FLAGS_BITS)

/**
 * @def ARCHI_POINTER_USER_FLAGS_MASK
 * @brief Mask of bits available for user-defined flags.
 */
#define ARCHI_POINTER_USER_FLAGS_MASK       \
    (((archi_pointer_flags_t)1 << ARCHI_POINTER_USER_FLAGS_BITS) - 1)

#endif // _ARCHIPELAGO_BASE_POINTER_TYP_H_

