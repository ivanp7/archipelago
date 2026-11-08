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
 * @brief The environment manager operations.
 */

#pragma once
#ifndef _ARCHI_ENV_I_MANAGER_FUN_H_
#define _ARCHI_ENV_I_MANAGER_FUN_H_

#include "archi_base/manager.typ.h"


/**
 * @brief Retrieve the environment manager pointer.
 *
 * If the environment manager pointer hasn't been initialized yet, the function returns NULL.
 *
 * @return Pointer to the environment manager.
 */
ARCHI_MANAGER_PTR_GET_FUNC(archi_manager_ptr_get__env);

/**
 * @brief Initialize the environment manager pointer.
 *
 * @pre
 *      manager != NULL
 *
 * @post
 *      Environment API is ready to be used.
 *
 * @note Subsequent invocations have no effect.
 *
 * @param[in] manager
 *      Environment manager.
 */
ARCHI_MANAGER_PTR_INIT_FUNC(archi_manager_ptr_init__env);

#endif // _ARCHI_ENV_I_MANAGER_FUN_H_

