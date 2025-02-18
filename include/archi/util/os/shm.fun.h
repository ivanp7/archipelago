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
 * @brief Shared memory operations.
 */

#pragma once
#ifndef _ARCHI_UTIL_OS_SHM_FUN_H_
#define _ARCHI_UTIL_OS_SHM_FUN_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Create and attach pointer-aware shared memory.
 *
 * The first object in the shared memory is the pointer to itself.
 * It is needed to attach the shared memory at the correct address,
 * so all pointers in the shared memory are valid.
 *
 * @return Shared memory address or NULL in case of failure.
 */
void*
archi_shared_memory_create(
        const char *pathname, ///< [in] Pathname associated with shared memory.
        int proj_id,          ///< [in] Project identifier associated with shared memory.

        size_t size ///< [in] Shared memory size.
);

/**
 * @brief Destroy shared memory.
 *
 * @return True on success, otherwise false.
 */
bool
archi_shared_memory_destroy(
        const char *pathname, ///< [in] Pathname associated with shared memory.
        int proj_id           ///< [in] Project identifier associated with shared memory.
);

/**
 * @brief Attach pointer-aware shared memory.
 *
 * The first object in the shared memory is the pointer to itself.
 * It must not be meddled with.
 *
 * @return Shared memory address or NULL in case of failure.
 */
void*
archi_shared_memory_attach(
        const char *pathname, ///< [in] Pathname associated with shared memory.
        int proj_id,          ///< [in] Project identifier associated with shared memory.

        bool writable ///< [in] Whether is shared memory attached for writing.
);

/**
 * @brief Detach shared memory.
 *
 * @return True on success, otherwise false.
 */
bool
archi_shared_memory_detach(
        const void *shmaddr ///< [in] Shared memory address.
);

#endif // _ARCHI_UTIL_OS_SHM_FUN_H_

