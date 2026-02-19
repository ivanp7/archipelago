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
 * @brief Context interface for arrays of data pointers.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_CTX_DPTR_ARRAY_VAR_H_
#define _ARCHI_CONTEXT_CTX_DPTR_ARRAY_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: array of pointers to data.
 *
 * Initialization parameters:
 * - "length" : (size_t) array length
 *
 * Getter slots:
 * - [index]        : array element #index
 * - "ptr" [index]  : (void*[]) array of pointers to array elements starting at #index
 * - "length"       : (size_t) array length
 *
 * Setter slots:
 * - [index]    : array element #index
 * - "length"   : (size_t) array length
 */
extern
const archi_context_interface_t
archi_context_interface__dptr_array;

#endif // _ARCHI_CONTEXT_CTX_DPTR_ARRAY_VAR_H_

