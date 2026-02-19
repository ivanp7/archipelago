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
#ifndef _ARCHI_ENV_API_CONTEXT_FUN_H_
#define _ARCHI_ENV_API_CONTEXT_FUN_H_

#include "archi_base/global.typ.h"


/**
 * @brief Initialize the environment context pointer.
 *
 * @pre
 *      context != NULL
 *
 * @post
 *      Environment API is ready to be used.
 *
 * @note Subsequent invocations have no effect.
 *
 * @param[in] context
 *      Environment context.
 */
ARCHI_GLOBAL_SET_FUNC(archi_global_context_set__env);

/**
 * @brief Retrieve the environment context pointer.
 *
 * If the environment context pointer hasn't been initialized yet, the function returns NULL.
 *
 * @return Pointer to the environment context.
 */
ARCHI_GLOBAL_GET_FUNC(archi_global_context__env);

#endif // _ARCHI_ENV_API_CONTEXT_FUN_H_

