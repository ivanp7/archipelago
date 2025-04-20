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
 * @brief Hierarchical state processor states for execution branching.
 */

#pragma once
#ifndef _ARCHI_HSP_STATE_BRANCH_FUN_H_
#define _ARCHI_HSP_STATE_BRANCH_FUN_H_

#include "archi/hsp/state/branch.typ.h"

/**
 * @brief State function for calling archi_hsp_advance().
 *
 * This state function expects archi_hsp_frame_t object as function data.
 */
ARCHI_HSP_STATE_FUNCTION(archi_hsp_state_advance);

/**
 * @brief Branch state function using a selector function.
 *
 * This state function expects archi_hsp_branch_state_data_t object as function data.
 */
ARCHI_HSP_STATE_FUNCTION(archi_hsp_state_branch);

/*****************************************************************************/

/**
 * @brief Select a branch unconditionally.
 *
 * Selector data is the returned branch index.
 * It must be less than the number of branches, otherwise 0 is returned.
 * If data is NULL, then 0 is returned.
 */
ARCHI_HSP_BRANCH_SELECTOR_FUNC(archi_hsp_branch_select_uncond);

/**
 * @brief Select a branch randomly.
 */
ARCHI_HSP_BRANCH_SELECTOR_FUNC(archi_hsp_branch_select_random);

#endif // _ARCHI_HSP_STATE_BRANCH_FUN_H_

