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
 * @brief Context interface for data of DEG node function for memory map-copy-unmap operation.
 */

#pragma once
#ifndef _ARCHI_MEMORY_DEG_CTX_COPY_DATA_VAR_H_
#define _ARCHI_MEMORY_DEG_CTX_COPY_DATA_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: data of HSP state for memory map-copy-unmap operation.
 *
 * Initialization parameters:
 * - "dest_memory"      : (archi_memory_t) destination memory
 * - "dest_map_data"    : destination mapping function data
 * - "dest_offset"      : (size_t) destination mapping offset
 * - "src_memory"       : (archi_memory_t) source memory
 * - "src_map_data"     : source mapping function data
 * - "src_offset"       : (size_t) source mapping offset
 * - "length"           : (size_t) number of data elements to copy
 *
 * Getter parameters:
 * - "dest.memory"      : (archi_memory_t) destination memory
 * - "dest.map_data"    : destination mapping function data
 * - "dest.offset"      : (size_t) destination mapping offset
 * - "src.memory"       : (archi_memory_t) source memory
 * - "src.map_data"     : source mapping function data
 * - "src.offset"       : (size_t) source mapping offset
 * - "length"           : (size_t) number of data elements to copy
 *
 * Setter parameters:
 * - "dest.memory"      : (archi_memory_t) destination memory
 * - "dest.map_data"    : destination mapping function data
 * - "dest.offset"      : (size_t) destination mapping offset
 * - "src.memory"       : (archi_memory_t) source memory
 * - "src.map_data"     : source mapping function data
 * - "src.offset"       : (size_t) source mapping offset
 * - "length"           : (size_t) number of data elements to copy
 */
extern
const archi_context_interface_t
archi_context_interface__deg_node_data__memory_map_copy_unmap;

#endif // _ARCHI_MEMORY_DEG_CTX_COPY_DATA_VAR_H_

