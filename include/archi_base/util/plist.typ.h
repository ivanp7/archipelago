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
 * @brief Types for key-value lists of parameters.
 */

#pragma once
#ifndef _ARCHI_BASE_UTIL_PLIST_TYP_H_
#define _ARCHI_BASE_UTIL_PLIST_TYP_H_

#include "archi_base/pointer.typ.h"
#include "archi_base/util/string.typ.h"
#include "archi_base/util/check.typ.h"

#include <stdbool.h>


/**
 * @brief Signature of a parameter value assignment function.
 */
#define ARCHI_PLIST_ASSIGN_FUNC(func_name)  void func_name( \
        void *dest, /* [in] Destination. */                 \
        size_t size, /* [in] Destination size in bytes. */  \
        archi_rcpointer_t value) /* [in] Parameter value. */

/**
 * @brief Parameter value assignment function type.
 */
typedef ARCHI_PLIST_ASSIGN_FUNC((*archi_plist_assign_func_t));

/**
 * @brief Description of a parameter in a key-value parameter list.
 *
 * `comp_fn` may be NULL, in which case `strcmp` is used by default.
 * `check_fn` may be NULL, in which case a value is accepted unconditionally.
 * `callback_fn` may be NULL, in which case nothing is done.
 */
typedef struct archi_plist_param {
    const char *name; ///< Parameter name.
    archi_string_comp_func_t comp_fn; ///< Name comparison function.

    struct {
        archi_value_check_func_t fn; ///< Function.
        const void *data; ///< Function data.
    } check; ///< Value check.

    struct {
        archi_plist_assign_func_t fn; ///< Function.
        void *to; ///< Destination.
        size_t sz; ///< Destination size in bytes.
        bool *flag; ///< Whether the parameter was set.
    } assign; ///< Value assignment.

    bool value_set; ///< Parameter was set.
    archi_rcpointer_t value; ///< Parameter value.
} archi_plist_param_t;

#endif // _ARCHI_BASE_UTIL_PLIST_TYP_H_

