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
 * @brief Built-in context interfaces.
 */

#pragma once
#ifndef _ARCHI_EXE_BUILTIN_VAR_H_
#define _ARCHI_EXE_BUILTIN_VAR_H_

#include "archi/app/context.typ.h"

/**
 * @brief Built-in context interfaces.
 */
extern
const archi_context_interface_t *const archi_builtin_interfaces[];

/**
 * @brief Aliases of built-in context interfaces.
 */
extern
const char *const archi_builtin_interfaces_aliases[];

/**
 * @brief Number of built-in context interfaces.
 */
extern
const size_t archi_builtin_interfaces_num_of;

#endif // _ARCHI_EXE_BUILTIN_VAR_H_

