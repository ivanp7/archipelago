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
 * @brief Operations on key-value lists of parameters.
 */

#pragma once
#ifndef _ARCHIPELAGO_UTIL_PARAMETERS_FUN_H_
#define _ARCHIPELAGO_UTIL_PARAMETERS_FUN_H_

#include "archipelago/util/parameters.typ.h"
#include "archipelago/util/string.typ.h"
#include "archipelago/base/kvlist.typ.h"
#include "archipelago/base/error.typ.h"

/**
 * @brief Parse key-value list of parameters.
 *
 * This function iterates `params` and fills `parsed` with values
 * of first occurences of the specified parameters.
 * Values in `params` are checked for compatibility with `parsed[i].value.attr`.
 * Parameters with `parsed[i].value_set == true` are ignored.
 * Names in `parsed` must not contain duplicates.
 * `comp_fn` may be NULL, in that case `strcmp` is used instead.
 * First argument to `comp_fn` is from `params`, second argument is from `parsed`.
 *
 * @return True on success, false on failure.
 */
bool
archi_kvlist_parameters_parse(
        const archi_kvlist_rc_t *params, ///< [in] Parameter list.
        archi_kvlist_parameter_t parsed[], ///< [in,out] Descriptions of parsed parameters.
        size_t num_parsed, ///< [in] Number of parameters to parse.
        bool ignore_unknown, ///< [in] Ignore unknown parameters.
        archi_string_comparison_func_t comp_fn, ///< [in] String comparison function.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

#endif // _ARCHIPELAGO_UTIL_PARAMETERS_FUN_H_

