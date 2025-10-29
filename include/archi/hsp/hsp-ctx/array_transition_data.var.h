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
 * @brief Context interface for HSP array transition data.
 */

#pragma once
#ifndef _ARCHI_HSP_HSP_CTX_ARRAY_TRANSITION_DATA_VAR_H_
#define _ARCHI_HSP_HSP_CTX_ARRAY_TRANSITION_DATA_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: HSP array transition data.
 *
 * Initialization parameters:
 * - "num_transitions"  : (size_t) number of transitions
 *
 * Getter slots:
 * - "num_transitions"      : (size_t) number of transitions
 * - "transition" [index]   : (archi_hsp_transition_t) transition #index
 *
 * Setter slots:
 * - "transition" [index]   : (archi_hsp_transition_t) transition #index
 */
extern
const archi_context_interface_t
archi_context_interface__hsp_transition_data__array;

#endif // _ARCHI_HSP_HSP_CTX_ARRAY_TRANSITION_DATA_VAR_H_

