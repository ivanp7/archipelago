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
 * @brief Data for DEG operation function for copying memory.
 */

#pragma once
#ifndef _ARCHI_MEMORY_EXE_COPY_TYP_H_
#define _ARCHI_MEMORY_EXE_COPY_TYP_H_

#include <stddef.h> // for size_t


/**
 * @brief Operation function data: copy memory.
 */
typedef struct archi_dexgraph_op_data__memory_copy {
    void *destination; ///< Destination memory.
    const void *source; ///< Source memory.

    size_t offset_dest; ///< Offset to destination area in data elements.
    size_t offset_src; ///< Offset to source area in data elements.

    size_t length; ///< Number of data elements to copy.
    size_t stride; ///< Size of a data element in bytes.
} archi_dexgraph_op_data__memory_copy_t;

#endif // _ARCHI_MEMORY_EXE_COPY_TYP_H_

