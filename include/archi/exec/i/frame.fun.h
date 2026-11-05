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
 * @brief Operations with directed execution graph frames.
 */

#pragma once
#ifndef _ARCHI_EXEC_I_FRAME_FUN_H_
#define _ARCHI_EXEC_I_FRAME_FUN_H_

#include "archi/exec/i/frame.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Execute the rest of node operations starting at the specified frame.
 *
 * The function calls non-NULL operation functions in the current node sequence,
 * starting from the specified index, halting if any function returns non-zero error code.
 *
 * @return Index of the first operation not executed (successfully).
 */
size_t
archi_dexgraph_execute_node(
        archi_dexgraph_frame_t frame, ///< [in] Frame.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Execute a directed graph starting at the specified frame.
 *
 * This function implements the most basic graph execution algorithm:
 * 1. execute the current node operation functions;
 * 2. execute the current node transition function;
 * 3. update the current node;
 * 4. repeat until the current node is NULL.
 *
 * Returned frame contains NULL node if execution terminated without errors.
 * Otherwise, it stores the current node pointer and operation index
 * of the location where an error occured.
 *
 * @return Execution frame at the halt point.
 */
archi_dexgraph_frame_t
archi_dexgraph_execute(
        archi_dexgraph_frame_t frame, ///< [in] Frame.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_EXEC_I_FRAME_FUN_H_

