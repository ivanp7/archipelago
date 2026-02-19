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
 * @brief Type of value check functions.
 */

#pragma once
#ifndef _ARCHI_BASE_UTIL_CHECK_TYP_H_
#define _ARCHI_BASE_UTIL_CHECK_TYP_H_

#include "archi_base/pointer.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


/**
 * @brief Signature of a value check function.
 *
 * @return True if the value is compatible, false otherwise.
 */
#define ARCHI_VALUE_CHECK_FUNC(func_name)   bool func_name( \
        archi_pointer_t value, /* [in] Value to check. */   \
        const void *data, /* [in] Function data. */         \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Value check function type.
 */
typedef ARCHI_VALUE_CHECK_FUNC((*archi_value_check_func_t));

#endif // _ARCHI_BASE_UTIL_CHECK_TYP_H_

