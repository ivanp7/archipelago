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
 * @brief Environmental variable operations.
 */

#pragma once
#ifndef _ARCHI_IPC_ENV_INTERFACE_FUN_H_
#define _ARCHI_IPC_ENV_INTERFACE_FUN_H_

#include "archi/util/status.typ.h"

/**
 * @brief Allocate and return a copy of an environment variable's value.
 *
 * This function looks up the environment variable named by @p name,
 * allocates a buffer to hold its null‑terminated value, and returns
 * a pointer to that buffer. The caller is responsible for freeing
 * the returned string via free().
 *
 * @note
 *   Internally uses a spinlock (C11 atomic_flag) to serialize calls to getenv()
 *   on platforms that support C11 atomics. Safe for concurrent use when atomics
 *   are available; otherwise, concurrent calls may race.
 *
 * @param[in]  name  Name of the environment variable to retrieve. Must not be NULL.
 * @param[out] code  Optional pointer to an archi_status_t to receive the status.
 *                   May be NULL.
 *
 * @return Pointer to a newly malloc'd C‑string containing the variable's value,
 *         or NULL on error (see @p code for the specific failure reason).
 *
 * Possible status codes written to *@p code (if @p code != NULL):
 *   - 0 : Value found and copied successfully.
 *   - 1 : Variable with such name is not present in the environment.
 *   - ARCHI_STATUS_EMISUSE   : @p name was NULL.
 *   - ARCHI_STATUS_ENOMEMORY : Memory allocation failed.
 *
 * @par Example
 * @code
 *   archi_status_t st;
 *   char *path = archi_env_get("PATH", &st);
 *   if (path) {
 *       printf("PATH = %s\n", path);
 *       free(path);
 *   } else {
 *       fprintf(stderr, "archi_env_get() failed: %d\n", st);
 *   }
 * @endcode
 */
char*
archi_env_get(
        const char *name,
        archi_status_t *code
);

#endif // _ARCHI_IPC_ENV_INTERFACE_FUN_H_

