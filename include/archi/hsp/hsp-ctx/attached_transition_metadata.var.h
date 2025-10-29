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
 * @brief Context interface for HSP state metadata for attached transitions.
 */

#pragma once
#ifndef _ARCHI_HSP_HSP_CTX_ATTACHED_TRANSITION_METADATA_VAR_H_
#define _ARCHI_HSP_HSP_CTX_ATTACHED_TRANSITION_METADATA_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: HSP state metadata for attached transitions.
 *
 * Initialization parameters:
 * - "pre_function"    : (archi_hsp_transition_function_t) prefix transition function
 * - "pre_data"        : prefix transition data
 * - "post_function"   : (archi_hsp_transition_function_t) postfix transition function
 * - "post_data"       : postfix transition data
 * - "metadata"        : other state metadata
 *
 * Getter slots:
 * - "pre.function"    : (archi_hsp_transition_function_t) prefix transition function
 * - "pre.data"        : prefix transition data
 * - "post.function"   : (archi_hsp_transition_function_t) postfix transition function
 * - "post.data"       : postfix transition data
 * - "metadata"        : other state metadata
 *
 * Setter slots:
 * - "pre.function"    : (archi_hsp_transition_function_t) prefix transition function
 * - "pre.data"        : prefix transition data
 * - "post.function"   : (archi_hsp_transition_function_t) postfix transition function
 * - "post.data"       : postfix transition data
 * - "metadata"        : other state metadata
 */
extern
const archi_context_interface_t
archi_context_interface__hsp_state_metadata__attached_transition;

#endif // _ARCHI_HSP_HSP_CTX_ATTACHED_TRANSITION_METADATA_VAR_H_

