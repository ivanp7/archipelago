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
 * @brief Context interface for HSP branch state data.
 */

#pragma once
#ifndef _ARCHI_HSP_HSP_CTX_BRANCH_STATE_DATA_VAR_H_
#define _ARCHI_HSP_HSP_CTX_BRANCH_STATE_DATA_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: HSP branch state data.
 *
 * Initialization parameters:
 * - "num_branches"         : (size_t) number of branches
 * - "selector_function"    : (archi_hsp_branch_selector_func_t) branch selector function
 * - "selector_data"        : branch selector data
 *
 * Getter slots:
 * - "num_branches"         : (size_t) number of branches
 * - "branch" [index]       : (archi_hsp_frame_t) branch #index
 * - "selector_function"    : (archi_hsp_branch_selector_func_t) branch selector function
 * - "selector_data"        : branch selector data
 *
 * Setter slots:
 * - "branch" [index]       : (archi_hsp_frame_t) branch #index
 * - "selector_function"    : (archi_hsp_branch_selector_func_t) branch selector function
 * - "selector_data"        : branch selector data
 */
extern
const archi_context_interface_t
archi_context_interface__hsp_state_data__branch;

#endif // _ARCHI_HSP_HSP_CTX_BRANCH_STATE_DATA_VAR_H_

