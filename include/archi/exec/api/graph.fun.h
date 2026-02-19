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
 * @brief Directed execution graph operations.
 */

#pragma once
#ifndef _ARCHI_EXEC_API_GRAPH_FUN_H_
#define _ARCHI_EXEC_API_GRAPH_FUN_H_

#include "archi/exec/api/frame.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


/**
 * @brief Directed execution graph execution mode.
 *
 * The greater the number -- the more interruptions will be.
 */
enum archi_dexgraph_exec_mode {
    ARCHI_DEXGRAPH__NO_INTERRUPT = 0,     ///< Execute without interruptions until halted (or error).
    ARCHI_DEXGRAPH__INTERRUPT_TRANSITION, ///< Execute interrupting at node transitions (or error).
    ARCHI_DEXGRAPH__INTERRUPT_OPERATION,  ///< Execute interrupting between operations (always).
};

/**
 * @brief Execute a directed graph starting at the specified frame.
 *
 * The algorithm steps:
 * 1. call non-null functions in the current node sequence, starting from the specified index,
 *    halting if any function returns non-zero error code;
 * 2. obtain branch index from the current node transition,
 *    halting if transition function returns non-zero error code;
 * 3. reset current node sequence index to zero;
 * 4. if branch index is equal to the special halt value, unset current node pointer and halt;
 * 5. update current node pointer and proceed to step 1 if the pointer is non-null, otherwise halt.
 *
 * Transitions to the next branch are done as follows:
 * 1. if transition function is not NULL, it is called with transition data to obtain branch index;
 * 2. otherwise, if transition data is not NULL, it is dereferenced to read branch index;
 * 3. in case both transition function and data are NULL, the default value of zero is used as branch index.
 *
 * Output frame context is empty if execution halted without error.
 * Otherwise, it contains the current node pointer and sequence index of the function where error occured.
 *
 * @return Execution frame at the interruption point.
 */
archi_dexgraph_frame_t
archi_dexgraph_execute(
        archi_dexgraph_frame_t frame, ///< [in] Frame.
        enum archi_dexgraph_exec_mode mode, ///< Execution mode.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_EXEC_API_GRAPH_FUN_H_

