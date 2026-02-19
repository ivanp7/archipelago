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
 * @brief Context interfaces for pointer wrappers.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_CTX_POINTER_VAR_H_
#define _ARCHI_CONTEXT_CTX_POINTER_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: arbitrary pointer.
 *
 * Initialization parameters:
 * - "pointee"  : entity to store
 *
 * Getter slots:
 * - "pointee"  : stored entity
 *
 * Setter slots:
 * - "pointee"  : entity to store
 */
extern
const archi_context_interface_t
archi_context_interface__pointer;

/**
 * @brief Context interface: pointer to data.
 *
 * Initialization parameters:
 * - "pointee"  : data to store
 * - "writable" : (char) data writability flag
 *
 * Getter slots:
 * - "pointee"  : stored data
 * - "writable" : (char) data writability flag
 *
 * Setter slots:
 * - "pointee"  : data to store
 * - "writable" : (char) data writability flag
 */
extern
const archi_context_interface_t
archi_context_interface__dpointer;

/**
 * @brief Context interface: pointer to primitive data.
 *
 * Initialization parameters:
 * - "pointee"      : data to store
 * - "offset"       : (long long) offset applied to the pointer
 * - "offset_unit"  : (size_t) unit of the offset applied to the pointer
 * - "writable"     : (char) data writability flag
 * - "length"       : (size_t) number of data elements
 * - "stride"       : (size_t) size of a data element in bytes
 * - "alignment"    : (size_t) data alignment requirement
 *
 * Getter slots:
 * - "pointee"      : stored data
 * - [offset]       : data stored at offset
 * - "writable"     : (char) data writability flag
 * - "length"       : (size_t) number of data elements
 * - "stride"       : (size_t) size of a data element in bytes
 * - "size"         : (size_t) total size of data in bytes
 * - "alignment"    : (size_t) data alignment requirement
 *
 * Calls:
 * - "shift_ptr"    : shift the pointer by arbitrary number of strides
 *      parameters:
 *        - "offset"    : (long long) offset within pointee in strides
 * - "set_attr"     : change length, stride, alignment requirement
 *      parameters:
 *        - "length"    : (size_t) number of data elements
 *        - "stride"    : (size_t) size of a data element in bytes
 *        - "alignment" : (size_t) data alignment requirement
 * - "copy"         : copy memory
 *      parameters:
 *        - "src"           : source
 *        - "src_offset"    : (size_t) offset within source
 *        - "offset"        : (size_t) offset within destination
 *        - "length"        : (size_t) number of destination strides to copy
 * - "fill"         : fill memory with pattern
 *      parameters:
 *        - "pattern"   : pattern
 *        - "offset"    : (size_t) offset within destination
 *        - "length"    : (size_t) number of strides to fill
 *
 * Setter slots:
 * - "pointee"      : data to store
 * - "writable"     : (char) data writability flag
 * - "length"       : (size_t) number of data elements
 * - "stride"       : (size_t) size of a data element in bytes (preserves total data size)
 * - "alignment"    : (size_t) data alignment requirement
 */
extern
const archi_context_interface_t
archi_context_interface__pdpointer;

/**
 * @brief Context interface: pointer to complex data.
 *
 * Initialization parameters:
 * - "pointee"      : data to store
 * - "offset"       : (long long) offset applied to the pointer
 * - "offset_unit"  : (size_t) unit of the offset applied to the pointer
 * - "writable"     : (char) data writability flag
 * - "tag"          : (archi_pointer_attr_t) data type tag
 *
 * Getter slots:
 * - "pointee"  : stored data
 * - "writable" : (char) data writability flag
 * - "tag"      : (archi_pointer_attr_t) data type tag
 *
 * Setter slots:
 * - "pointee"  : data to store
 * - "writable" : (char) data writability flag
 * - "tag"      : (archi_pointer_attr_t) data type tag
 */
extern
const archi_context_interface_t
archi_context_interface__cdpointer;

/**
 * @brief Context interface: pointer to function.
 *
 * Initialization parameters:
 * - "pointee"  : function to store
 * - "tag"      : (archi_pointer_attr_t) function type tag
 *
 * Getter slots:
 * - "pointee"  : stored function
 * - "tag"      : (archi_pointer_attr_t) function type tag
 *
 * Setter slots:
 * - "pointee"  : function to store
 * - "tag"      : (archi_pointer_attr_t) function type tag
 */
extern
const archi_context_interface_t
archi_context_interface__fpointer;

#endif // _ARCHI_CONTEXT_CTX_POINTER_VAR_H_

