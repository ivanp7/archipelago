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
 * @brief Data for DEG operation function for mapping/unmapping memory.
 */

#pragma once
#ifndef _ARCHI_MEMORY_EXE_MAP_TYP_H_
#define _ARCHI_MEMORY_EXE_MAP_TYP_H_

#include "archi/memory/api/handle.typ.h"
#include "archi/exec/api/operation.typ.h"


/**
 * @brief Operation function data: wrapper for mapping and unmapping memory object.
 */
typedef struct archi_dexgraph_op_data__memory_map_unmap_wrapper {
    archi_memory_t memory; ///< Memory object.
    void *map_data; ///< Interface-specific data for mapping memory.
    size_t map_offset; ///< Offset of mapped region in data elements.
    size_t map_length; ///< Number of mapped data elements.

    archi_dexgraph_operation_t target; ///< Wrapped operation function.

    void **ptr; ///< Location to write pointer to mapped memory pointer to.
} archi_dexgraph_op_data__memory_map_unmap_wrapper_t;

#endif // _ARCHI_MEMORY_EXE_MAP_TYP_H_

