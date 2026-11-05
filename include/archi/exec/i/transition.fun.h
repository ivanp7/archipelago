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
 * @brief Functions of directed execution graph transitions.
 */

#pragma once
#ifndef _ARCHI_EXEC_API_TRANSITION_FUN_H_
#define _ARCHI_EXEC_API_TRANSITION_FUN_H_

#include "archi/exec/i/transition.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Obtain the transition successor node.
 *
 * If the transition function is not NULL, it is called with the transition data
 * to obtain the successor node.
 * Otherwise, the transition data is interpreted as the pointer to the successor node.
 *
 * @return The successor node, or NULL.
 */
const struct archi_dexgraph_node*
archi_dexgraph_transition_target(
        archi_dexgraph_transition_t transition, ///< [in] Transition.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_EXEC_API_TRANSITION_FUN_H_

