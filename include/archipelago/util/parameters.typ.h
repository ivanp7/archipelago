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
#ifndef _ARCHIPELAGO_UTIL_PARAMETERS_TYP_H_
#define _ARCHIPELAGO_UTIL_PARAMETERS_TYP_H_

#include "archipelago/base/pointer.typ.h"

#include <stdbool.h>

/**
 * @brief Key-value list parameter description.
 */
typedef struct archi_kvlist_parameter {
    const char *name; ///< Parameter name.
    archi_rcpointer_t value; ///< Parameter value.
    bool value_unchecked; ///< Whether parameter value attributes check is skipped.
    bool value_set; ///< Whether parameter value was set.
} archi_kvlist_parameter_t;

#endif // _ARCHIPELAGO_UTIL_PARAMETERS_TYP_H_

