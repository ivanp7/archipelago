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
 * @brief Types for strings.
 */

#pragma once
#ifndef _ARCHIPELAGO_UTIL_STRING_TYP_H_
#define _ARCHIPELAGO_UTIL_STRING_TYP_H_

/**
 * @brief Declare/define string comparison function.
 *
 * This function returns an integer indicating the result of the comparison, as follows:
 * - 0, if the `lhs` and `rhs` are equal;
 * - a negative value if `lhs` is less than `rhs`;
 * - a positive value if `lhs` is greater than `rhs`.
 *
 * @return Comparison result.
 */
#define ARCHI_STRING_COMPARISON_FUNC(name)  int name(   \
        const char *lhs, /* [in] Left hand side. */     \
        const char *rhs) /* [in] Left hand side. */

/**
 * @brief String comparison function type.
 */
typedef ARCHI_STRING_COMPARISON_FUNC((*archi_string_comparison_func_t));

#endif // _ARCHIPELAGO_UTIL_STRING_TYP_H_

