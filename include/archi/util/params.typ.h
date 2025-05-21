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
 * @brief The type of list of named values.
 */

#pragma once
#ifndef _ARCHI_UTIL_PARAMS_TYP_H_
#define _ARCHI_UTIL_PARAMS_TYP_H_

#include "archi/util/pointer.typ.h"

struct archi_parameter_list;

/**
 * @brief List of named values.
 */
typedef struct archi_parameter_list {
    struct archi_parameter_list *next; ///< Pointer to the next list node.

    const char *name;      ///< Parameter name.
    archi_pointer_t value; ///< Parameter value.
} archi_parameter_list_t;

#endif // _ARCHI_UTIL_PARAMS_TYP_H_

