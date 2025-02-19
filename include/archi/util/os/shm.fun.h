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

#include "archi/util/os/shm.typ.h"

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Open a memory-mapped file.
 *
 * @return File descriptor.
 */
int
archi_shm_open_file(
        const char *pathname, ///< Pathname of a memory-mapped file.

        bool readable, ///< [in] Whether is shared memory readable.
        bool writable  ///< [in] Whether is shared memory writable.
);

/**
 * @brief Close a memory-mapped object.
 *
 * @return File descriptor.
 */
bool
archi_shm_close(
        int fd ///< [in] File descriptor of the mapped object.
);

/**
 * @brief Map a pointer-aware memory-mapped file.
 *
 * The first object in the shared memory is void* pointer,
 * which must be equal to its own location address.
 * Pointers in the memory will be invalid if that's not the case.
 *
 * @return Shared memory address or NULL in case of failure.
 */
archi_shm_header_t*
archi_shm_map(
        int fd, ///< [in] File descriptor of the mapped object.

        bool readable, ///< [in] Whether is shared memory readable.
        bool writable, ///< [in] Whether is shared memory writable.
        bool shared,   ///< [in] Whether updates to the mapping are visible to other processes.
        int flags      ///< [in] Other mmap() flags.
);

/**
 * @brief Unmap a pointer-aware memory-mapped file.
 *
 * @return True on success, otherwise false.
 */
bool
archi_shm_unmap(
        archi_shm_header_t *shm ///< [in] Shared memory header.
);

#endif // _ARCHI_UTIL_OS_SHM_FUN_H_

