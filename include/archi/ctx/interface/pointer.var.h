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
 * @brief Context interface for pointer wrappers.
 */

#pragma once
#ifndef _ARCHI_CTX_INTERFACE_POINTER_VAR_H_
#define _ARCHI_CTX_INTERFACE_POINTER_VAR_H_

#include "archi/ctx/interface.typ.h"

/**
 * @brief Pointer initialization function.
 *
 * Accepts the following parameters:
 * - "value"        : pointer to store
 * - "flags"        : new pointer flags
 * - "layout"       : new array layout
 * - "num_elements" : new number of elements
 * - "element_size" : new element size
 * - "element_alignment" : new element alignment requirement
 */
ARCHI_CONTEXT_INIT_FUNC(archi_context_pointer_init);

/**
 * @brief Pointer finalization function.
 */
ARCHI_CONTEXT_FINAL_FUNC(archi_context_pointer_final);

/**
 * @brief Pointer getter function.
 *
 * Provides the following slots:
 * - "" [offset] : stored pointer + (offset) * (data element size)
 * - "flags"        : pointer flags
 * - "layout"       : array layout
 * - "num_elements" : number of elements
 * - "element_size" : element size
 * - "element_alignment" : element alignment requirement
 */
ARCHI_CONTEXT_GET_FUNC(archi_context_pointer_get);

/**
 * @brief Pointer setter function.
 *
 * Accepts the following slots:
 * - "value" : pointer to store
 * - "" [offset] : copy contents to stored pointer + (offset) * (data element size)
 */
ARCHI_CONTEXT_SET_FUNC(archi_context_pointer_set);

/**
 * @brief Pointer action function.
 *
 * Implements the following actions:
 * - "update" : update the stored pointer
 *      parameters:
 *        - "value"        : pointer to store
 *        - "flags"        : new pointer flags
 *        - "layout"       : new array layout
 *        - "num_elements" : new number of elements
 *        - "element_size" : new element size
 *        - "element_alignment" : new element alignment requirement
 */
ARCHI_CONTEXT_ACT_FUNC(archi_context_pointer_act);

/**
 * @brief Pointer interface.
 */
extern
const archi_context_interface_t archi_context_pointer_interface;

#endif // _ARCHI_CTX_INTERFACE_POINTER_VAR_H_

