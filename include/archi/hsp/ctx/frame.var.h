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
 * @brief Context interface for hierarchical state processor frames.
 */

#pragma once
#ifndef _ARCHI_HSP_CTX_FRAME_VAR_H_
#define _ARCHI_HSP_CTX_FRAME_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: HSP frame.
 *
 * Initialization parameters:
 * - "num_states"   : (size_t) number of states in the frame
 *
 * Getter slots:
 * - "num_states"               : (size_t) number of states in the frame
 * - "state.function" [index]   : (archi_hsp_state_function_t) function of state #index
 * - "state.data" [index]       : data of state #index
 * - "state.metadata" [index]   : metadata of state #index
 *
 * Calls:
 * - "execute"  : execute HSP with the frame as initial
 *      parameters:
 *        - "transition"    : (archi_hsp_transition_t) transition
 *
 * Setter slots:
 * - "state.function" [index]   : (archi_hsp_state_function_t) function of state #index
 * - "state.data" [index]       : data of state #index
 * - "state.metadata" [index]   : metadata of state #index
 */
extern
const archi_context_interface_t
archi_context_interface__hsp_frame;

#endif // _ARCHI_HSP_CTX_FRAME_VAR_H_

