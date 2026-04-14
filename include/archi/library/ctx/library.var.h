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
 * @brief Context interface for shared libraries.
 */

#pragma once
#ifndef _ARCHI_LIBRARY_CTX_LIBRARY_VAR_H_
#define _ARCHI_LIBRARY_CTX_LIBRARY_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: shared library.
 *
 * Initialization parameters:
 * - "pathname"     : (char[]) pathname of loaded library
 * - "params"       : (archi_library_load_params_t) library loading parameters structure
 * - "lazy"         : (char) lazy binding is performed
 * - "global"       : (char) defined symbols are available to subsequently loaded libraries
 * - "flags"        : (int) other library loading flags
 *
 * Getters:
 * - "data.***"     : data symbol named ***
 * - "function.***" : function symbol named ***
 *
 * Calls:
 * - "data.***"     : data symbol named ***
 *      parameters:
 *        - "tag"       : (archi_pointer_attr_t) data type tag
 *        - "length"    : (size_t) number of data elements
 *        - "stride"    : (size_t) size of a data element in bytes
 *        - "alignment" : (size_t) data alignment requirement in bytes
 * - "function.***" : function symbol named ***
 *      parameters:
 *        - "tag"       : (archi_pointer_attr_t) function type tag
 */
extern
const archi_context_interface_t
archi_context_interface__library;

#endif // _ARCHI_LIBRARY_CTX_LIBRARY_VAR_H_

