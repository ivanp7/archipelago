
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
 * @brief Context interface for directed execution graph operation sequences.
 */

#pragma once
#ifndef _ARCHI_EXEC_CTX_OPERATION_VAR_H_
#define _ARCHI_EXEC_CTX_OPERATION_VAR_H_

#include "archi/context/i/interface.typ.h"


/**
 * @brief Context interface: directed execution graph operation sequence.
 *
 * Initialization parameters:
 * - "length"   : (size_t) number of operations in the sequence
 *
 * Getter slots:
 * - "length"           : (size_t) number of operations in the sequence
 * - "function" [index] : (archi_dexgraph_operation_func_t) operation function #index
 * - "data" [index]     : data of operation function #index
 *
 * Setter slots:
 * - "function" [index] : (archi_dexgraph_operation_func_t) operation function #index
 * - "data" [index]     : data of operation function #index
 */
extern
const archi_context_interface_t
archi_context_interface__dexgraph_operation_seq;

#endif // _ARCHI_EXEC_CTX_OPERATION_VAR_H_

