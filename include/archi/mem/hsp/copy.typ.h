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
 * @brief Types for memory copying HSP state.
 */

#pragma once
#ifndef _ARCHI_MEM_HSP_COPY_TYP_H_
#define _ARCHI_MEM_HSP_COPY_TYP_H_

#include "archi/mem/api/interface.fun.h"

/**
 * @brief Parameters for archi_memory_map_copy_unmap().
 */
typedef struct archi_memory_map_copy_unmap_data {
    archi_memory_t memory_dest; ///< Destination memory.
    size_t offset_dest; ///< Offset into destination memory in data elements.
    void *map_data_dest; ///< Interface-specific data for mapping destination memory.

    archi_memory_t memory_src; ///< Source memory.
    size_t offset_src; ///< Offset into source memory in data elements.
    void *map_data_src; ///< Interface-specific data for mapping source memory.

    size_t num_of; ///< Number of data elements to copy.
} archi_memory_map_copy_unmap_data_t;

#endif // _ARCHI_MEM_HSP_COPY_TYP_H_

