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
 * @brief Data for DEG node functions for memory copying.
 */

#pragma once
#ifndef _ARCHI_MEMORY_DEG_COPY_TYP_H_
#define _ARCHI_MEMORY_DEG_COPY_TYP_H_

#include "archi/memory/api/handle.typ.h"

/**
 * @brief Data for archi_deg_node__memory_map_copy_unmap().
 */
typedef struct archi_deg_node_data__memory_map_copy_unmap {
    struct {
        archi_memory_t memory; ///< Memory object.
        void *map_data; ///< Interface-specific data for mapping memory.
        size_t offset; ///< Offset into memory in data elements.
    } dest,  ///< Destination memory.
        src; ///< Source memory.

    size_t length; ///< Number of data elements to copy.
} archi_deg_node_data__memory_map_copy_unmap_t;

#endif // _ARCHI_MEMORY_DEG_COPY_TYP_H_

