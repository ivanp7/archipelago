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
#ifndef _ARCHI_BASE_UTIL_PLIST_FUN_H_
#define _ARCHI_BASE_UTIL_PLIST_FUN_H_

#include "archi_base/util/plist.typ.h"
#include "archi_base/kvlist.typ.h"


/**
 * @brief Parse key-value list of parameters.
 *
 * This function iterates a parameter list and stores the first value of each parameter
 * into the array of descriptions.
 *
 * @return True on success, false on failure.
 */
bool
archi_plist_parse(
        const archi_kvlist_t *params, ///< [in] Parameter list.
        bool with_ref_count, ///< [in] Whether values in the list are reference-counted.
        archi_plist_param_t parsed[], ///< [in,out] NULL-terminated array of parameter descriptions.
        bool ignore_unknown, ///< [in] Ignore unknown parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/*****************************************************************************/

/**
 * @brief Parameter value assignment: boolean.
 */
ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__bool);

/**
 * @brief Parameter value assignment: value.
 */
ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__value);

/**
 * @brief Parameter value assignment: data pointer.
 */
ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__dptr);

/**
 * @brief Parameter value assignment: data pointer (nullify empty).
 */
ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__dptr_n);

/**
 * @brief Parameter value assignment: function pointer.
 */
ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__fptr);

/**
 * @brief Parameter value assignment: archi_pointer_t.
 */
ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__pointer);

/**
 * @brief Parameter value assignment: archi_rcpointer_t.
 */
ARCHI_PLIST_ASSIGN_FUNC(archi_plist_assign__rcpointer);

#endif // _ARCHI_BASE_UTIL_PLIST_FUN_H_

