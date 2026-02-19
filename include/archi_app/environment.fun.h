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
 * @brief Operations on the global environment context.
 */

#pragma once
#ifndef _ARCHI_APP_ENVIRONMENT_FUN_H_
#define _ARCHI_APP_ENVIRONMENT_FUN_H_

#include "archi_base/global.typ.h"

#include <stdbool.h>


/**
 * @brief Retrieve the global environment context.
 *
 * @return Pointer to the global environment context.
 */
ARCHI_GLOBAL_GET_FUNC(archi_app_env_global_context);

/*****************************************************************************/

/**
 * @brief Initialize the global environment context.
 *
 * @return True on success, false on failure.
 */
bool
archi_app_env_initialize(void);

/**
 * @brief Finalize the global environment context.
 */
void
archi_app_env_finalize(void);

#endif // _ARCHI_APP_ENVIRONMENT_FUN_H_

