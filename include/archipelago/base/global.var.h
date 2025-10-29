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
 * @brief Array of descriptions of built-in global objects.
 */

#pragma once
#ifndef _ARCHIPELAGO_BASE_GLOBAL_VAR_H_
#define _ARCHIPELAGO_BASE_GLOBAL_VAR_H_

#include "archipelago/base/global.typ.h"

/**
 * @brief NULL-terminated array of global object initialization specifications.
 */
extern
const archi_global_init_spec_t
archi_global_init_specs[];

#endif // _ARCHIPELAGO_BASE_GLOBAL_VAR_H_

