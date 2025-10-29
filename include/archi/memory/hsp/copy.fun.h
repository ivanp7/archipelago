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
 * @brief HSP state for copying data between memory objects.
 */

#pragma once
#ifndef _ARCHI_MEMORY_HSP_COPY_FUN_H_
#define _ARCHI_MEMORY_HSP_COPY_FUN_H_

#include "archi/hsp/api/state.typ.h"

/**
 * @brief Copy a chunk of data between two memory regions.
 *
 * State function data type: archi_hsp_state_data__memory_map_copy_unmap_t.
 *
 * This function performs the following steps:
 *   1. If copy_length == 0, it is set to dest_length - dest_offset.
 *   2. Validate that both source and destination memory are non‐NULL, have NULL mapping,
 *   have matching element sizes and alignment, and the copied regions are valid.
 *   3. Map the source region for access via archi_memory_map().
 *   4. Map the destination region for access via archi_memory_map().
 *      If mapping the destination fails, the source mapping is undone.
 *   5. memcpy() num_of data elements from source->mapping.ptr to destination->mapping.ptr.
 *   6. Unmap the destination, then unmap the source via archi_memory_unmap().
 */
ARCHI_HSP_STATE_FUNCTION(archi_hsp_state__memory_map_copy_unmap);

#endif // _ARCHI_MEMORY_HSP_COPY_FUN_H_

