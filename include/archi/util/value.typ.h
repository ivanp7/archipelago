/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Value type.
 */

#pragma once
#ifndef _ARCHI_UTIL_VALUE_TYP_H_
#define _ARCHI_UTIL_VALUE_TYP_H_

#include <stddef.h>

/**
 * @brief Function pointer wrapper.
 */
typedef struct archi_fptr_wrapper {
    void (*fptr)(void); ///< Pointer to function.
} archi_fptr_wrapper_t;

/*****************************************************************************/

/**
 * @brief Value type.
 */
typedef enum archi_value_type {
    ARCHI_VALUE_NULL = 0,   ///< No value.

    ARCHI_VALUE_FALSE,      ///< Falsey boolean value.
    ARCHI_VALUE_TRUE,       ///< Truthy boolean value.

    ARCHI_VALUE_UINT,       ///< Unsigned integer.
    ARCHI_VALUE_SINT,       ///< Signed integer.
    ARCHI_VALUE_FLOAT,      ///< Floating-point number.

    ARCHI_VALUE_POINTER,    ///< Pointer to data or a function.

    ARCHI_VALUE_STRING,     ///< Null-terminated string.
    ARCHI_VALUE_DATA,       ///< Binary data.

    ARCHI_VALUE_NESTED,     ///< Nested node.
    ARCHI_VALUE_LIST,       ///< Nested list.
} archi_value_type_t;

/**
 * @brief Value pointer with metadata.
 *
 * Minimum size of memory pointed to by ptr is (size * num_of).
 */
typedef struct archi_value {
    void *ptr; ///< Pointer to the first value element.

    size_t size; ///< Size of a value element, or zero if unknown.
    size_t num_of; ///< Number of value elements.

    archi_value_type_t type; ///< Value element type.
} archi_value_t;

#endif // _ARCHI_UTIL_VALUE_TYP_H_

