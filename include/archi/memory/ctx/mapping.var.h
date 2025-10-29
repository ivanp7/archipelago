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
 * @brief Context interface for memory mapping objects.
 */

#pragma once
#ifndef _ARCHI_MEMORY_CTX_MAPPING_VAR_H_
#define _ARCHI_MEMORY_CTX_MAPPING_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: memory mapping.
 *
 * Initialization parameters:
 * - "memory"   : (archi_memory_t) pointer to memory object to map
 * - "map_data" : data for mapping function
 * - "offset"   : (size_t) offset of mapped region
 * - "length"   : (size_t) length of mapped region
 *
 * Getter slots:
 * - "memory"   : (archi_memory_t) pointer to backing memory object
 * - "ptr"      : pointer to mapped memory
 * - "offset"   : (size_t) offset of mapped region
 * - "length"   : (size_t) length of mapped region
 * - "size"     : (size_t) size of mapped region in bytes
 */
extern
const archi_context_interface_t
archi_context_interface__memory_mapping;

#endif // _ARCHI_MEMORY_CTX_MAPPING_VAR_H_

