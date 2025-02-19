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
 * @brief Types for shared memory operations.
 */

#pragma once
#ifndef _ARCHI_UTIL_OS_SHM_TYP_H_
#define _ARCHI_UTIL_OS_SHM_TYP_H_

/**
 * @brief Shared memory header.
 *
 * If header is an object in shared memory,
 * header.shmaddr must be equal to &header.
 *
 * (header.shmend - header.shmaddr) is the full size of shared memory.
 */
typedef struct archi_shm_header {
    void *shmaddr; ///< Address of the shared memory starting location.
    void *shmend;  ///< Address of the first location beyond shared memory end.
} archi_shm_header_t;

#endif // _ARCHI_UTIL_OS_SHM_TYP_H_

