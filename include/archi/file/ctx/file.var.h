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
 * @brief Context interface for files.
 */

#pragma once
#ifndef _ARCHI_FILE_CTX_FILE_VAR_H_
#define _ARCHI_FILE_CTX_FILE_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: file.
 *
 * Initialization parameters:
 * - "pathname"     : (char[]) pathname of file to open
 * - "fd"           : (archi_file_descriptor_t) file descriptor to duplicate
 * - "params"       : (archi_file_open_params_t) file opening parameters structure
 * - "size"         : (size_t) size of created file
 * - "readable"     : (char) file is opened for reading
 * - "writable"     : (char) file is opened for writing
 * - "create"       : (char) file creation request
 * - "exclusive"    : (char) file is created only, not opened
 * - "truncate"     : (char) existing file is truncated
 * - "append"       : (char) file is opened in append mode
 * - "flags"        : (int) other file opening flags
 * - "mode"         : (int) file creation mode (permissions)
 * - "stream"       : (char) open file stream using the new descriptor
 *
 * Getter slots:
 * - "fd"       : (archi_file_descriptor_t) file descriptor
 * - "offset"   : (long long) file offset (in bytes) from file beginning
 *
 * Calls:
 * - "read"     : read data from file to memory
 *      parameters:
 *        - "dest"          : destination
 *        - "dest_offset"   : (size_t) offset within destination
 *        - "length"        : (size_t) number of strides to read
 * - "write"    : write data to file from memory
 *      parameters:
 *        - "src"           : source
 *        - "src_offset"    : (size_t) offset within source
 *        - "length"        : (size_t) number of strides to write
 * - "sync"     : synchronize file state with storage
 *
 * Setter slots:
 * - "offset"       : (long long) file offset (in bytes) from file beginning
 * - "offset.end"   : (long long) file offset (in bytes) from file end
 * - "offset.shift" : (long long) file offset (in bytes) from the current position
 */
extern
const archi_context_interface_t
archi_context_interface__file;

#endif // _ARCHI_FILE_CTX_FILE_VAR_H_

