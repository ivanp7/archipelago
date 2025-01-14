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
 * @brief Operations on S-expressions.
 */

#pragma once
#ifndef _ARCHI_SEXP_FUN_H_
#define _ARCHI_SEXP_FUN_H_

#include <stdio.h> // for FILE

struct archi_sexp;
struct archi_sexp_cons;

/**
 * @brief Parse S-expression from a stream.
 *
 * @return Parsed S-expression.
 */
struct archi_sexp*
archi_sexp_parse_stream(
        FILE *stream, ///< [in] Stream to read and parse S-expression from.

        size_t *error_line,  ///< [out] Line number where the first error occured.
        size_t *error_column ///< [out] Column on the line where the first error occured.
);

/**
 * @brief Destroy whole S-expression tree.
 *
 * All elements in the tree are assumed to be allocated on the heap.
 */
void
archi_sexp_destroy_tree(
        struct archi_sexp *sexp ///< [in] S-expression to destroy.
);

#endif // _ARCHI_SEXP_FUN_H_

