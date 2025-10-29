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
 * @brief Context interface for environment variables.
 */

#pragma once
#ifndef _ARCHI_ENV_CTX_VARIABLE_VAR_H_
#define _ARCHI_ENV_CTX_VARIABLE_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: environment variable.
 *
 * Initialization parameters:
 * - "default_value"    : default value of environment variables
 *
 * Getter slots: any (without indices only) -- find an environment variable
 * with the specified name.
 *
 * Setter slots:
 * - "default_value"    : default value of environment variables
 */
extern
const archi_context_interface_t
archi_context_interface__env_variable;

#endif // _ARCHI_ENV_CTX_VARIABLE_VAR_H_

