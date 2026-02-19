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
 * @brief Context interface for memory objects.
 */

#pragma once
#ifndef _ARCHI_MEMORY_CTX_MEMORY_VAR_H_
#define _ARCHI_MEMORY_CTX_MEMORY_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: memory.
 *
 * Initialization parameters:
 * - "interface"        : (archi_memory_interface_t) pointer to memory interface
 * - "alloc_data"       : data for allocation function
 * - "length"           : (size_t) number of data elements to allocate
 * - "stride"           : (size_t) size of a data element in bytes
 * - "alignment"        : (size_t) alignment requirement of a data element in bytes
 * - "ext_alignment"    : (size_t) extended alignment requirement of memory in bytes
 *
 * Getter slots:
 * - "interface"        : (archi_memory_interface_t) pointer to memory interface
 * - "allocation"       : pointer to memory allocation
 * - "length"           : (size_t) number of data elements
 * - "stride"           : (size_t) size of a data element in bytes
 * - "size"             : (size_t) size of the memory in bytes
 * - "alignment"        : (size_t) alignment requirement of a data element in bytes
 * - "ext_alignment"    : (size_t) extended alignment requirement of memory in bytes
 */
extern
const archi_context_interface_t
archi_context_interface__memory;

#endif // _ARCHI_MEMORY_CTX_MEMORY_VAR_H_

