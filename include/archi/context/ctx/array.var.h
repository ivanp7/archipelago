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
 * @brief Context interface for arrays of pointers.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_CTX_ARRAY_VAR_H_
#define _ARCHI_CONTEXT_CTX_ARRAY_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Array initialization function.
 *
 * Accepts the following parameters:
 * - "num_elements" : number of elements
 * - "flags"        : array flags
 * - "func_ptrs"    : whether pointers to functions are stored
 */
ARCHI_CONTEXT_INIT_FUNC(archi_context_array_init);

/**
 * @brief Array finalization function.
 */
ARCHI_CONTEXT_FINAL_FUNC(archi_context_array_final);

/**
 * @brief Array getter function.
 *
 * Provides the following slots:
 * - "" [index] : element #index
 * - "elements"  : array of references to separate elements of the array
 */
ARCHI_CONTEXT_GET_FUNC(archi_context_array_get);

/**
 * @brief Array setter function.
 *
 * Accepts the following slots:
 * - "" [index] : element #index
 * - "num_elements" : number of elements
 */
ARCHI_CONTEXT_SET_FUNC(archi_context_array_set);

/**
 * @brief Array interface.
 */
extern
const archi_context_interface_t archi_context_array_interface;

#endif // _ARCHI_CONTEXT_CTX_ARRAY_VAR_H_

