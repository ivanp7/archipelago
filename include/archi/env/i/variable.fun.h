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
 * @brief Environment variable operations.
 */

#pragma once
#ifndef _ARCHI_ENV_API_VARIABLE_FUN_H_
#define _ARCHI_ENV_API_VARIABLE_FUN_H_

#include "archi_base/error.typ.h"


/**
 * @brief Allocate and return a copy of value of an environment variable.
 *
 * This function looks up the environment variable named by @p name,
 * allocates a buffer to hold its null‑terminated value, and returns
 * a pointer to that buffer. The caller is responsible for freeing
 * the returned string via free().
 *
 * @note
 *   Internally uses a mutex to serialize calls to getenv()
 *   on platforms that support threading. Safe for concurrent use.
 *
 * @return Environment variable value, or NULL if variable with such name isn't found.
 */
char*
archi_env_get(
        const char *name, ///< [in] Environment variable name.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_ENV_API_VARIABLE_FUN_H_

