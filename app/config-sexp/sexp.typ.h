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
 * @brief Types for S-expressions.
 */

#pragma once
#ifndef _ARCHI_SEXP_TYP_H_
#define _ARCHI_SEXP_TYP_H_

struct archi_sexp;

/**
 * @brief Cons-cell.
 */
typedef struct archi_sexp_cons {
    struct archi_sexp *first; ///< Head of the list.
    struct archi_sexp *rest;  ///< Rest of the list (all but head).
} archi_sexp_cons_t;

typedef unsigned long long archi_sexp_uint_t;    ///< Unsigned integer.
typedef long long archi_sexp_sint_t;             ///< Signed integer.
typedef long double archi_sexp_float_t;          ///< Floating-point number.
typedef char *archi_sexp_string_t;               ///< String.
typedef archi_sexp_string_t archi_sexp_symbol_t; ///< Symbol.

/**
 * @brief S-expression type.
 */
typedef enum archi_sexp_type {
    ARCHI_SEXP_TYPE_NIL = 0, ///< Nil (no value/empty list).
    ARCHI_SEXP_TYPE_CONS,    ///< Cons-cell.

    // Atoms
    ARCHI_SEXP_TYPE_UINT,    ///< Unsigned integer.
    ARCHI_SEXP_TYPE_SINT,    ///< Signed integer.
    ARCHI_SEXP_TYPE_FLOAT,   ///< Floating-point number.
    ARCHI_SEXP_TYPE_STRING,  ///< String.
    ARCHI_SEXP_TYPE_SYMBOL,  ///< Symbol.
} archi_sexp_type_t;

/**
 * @brief S-expression.
 */
typedef struct archi_sexp {
    union {
        archi_sexp_cons_t as_cons;      ///< Cons-cell.

        archi_sexp_uint_t as_uint;      ///< Unsigned integer.
        archi_sexp_sint_t as_sint;      ///< Signed integer.
        archi_sexp_float_t as_float;    ///< Floating-point number.
        archi_sexp_string_t as_string;  ///< String.
        archi_sexp_symbol_t as_symbol;  ///< Symbol.
    };

    archi_sexp_type_t type; ///< S-expression type.
} archi_sexp_t;

#endif // _ARCHI_SEXP_TYP_H_

