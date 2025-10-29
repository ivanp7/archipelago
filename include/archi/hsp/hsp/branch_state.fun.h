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
 * @brief Hierarchical state processor states for execution branching.
 */

#pragma once
#ifndef _ARCHI_HSP_HSP_BRANCH_STATE_FUN_H_
#define _ARCHI_HSP_HSP_BRANCH_STATE_FUN_H_

#include "archi/hsp/hsp/branch_state.typ.h"

/**
 * @brief Allocate branch state data.
 *
 * @note The returned pointer must be eventually released using free().
 *
 * @return Newly allocated branch state data.
 */
archi_hsp_state_data__branch_t*
archi_hsp_state_data_alloc__branch(
        size_t num_branches, ///< [in] Number of branches.
        archi_hsp_branch_selector_func_t selector_fn, ///< [in] Branch selector function.
        void *selector_data ///< [in] Branch selector data.
);

/*****************************************************************************/

/**
 * @brief State function for calling archi_hsp_advance().
 *
 * State function data type: archi_hsp_frame_t.
 */
ARCHI_HSP_STATE_FUNCTION(archi_hsp_state__advance);

/**
 * @brief Branch state function using a selector function to choose a branch.
 *
 * State function data type: archi_hsp_state_data__branch_t.
 */
ARCHI_HSP_STATE_FUNCTION(archi_hsp_state__branch);

/*****************************************************************************/

/**
 * @brief Select a branch unconditionally.
 *
 * Selector function data type: size_t.
 * Selector data is the returned branch index.
 * If data is NULL, then 0 is returned.
 */
ARCHI_HSP_BRANCH_SELECTOR_FUNC(archi_hsp_branch_select__uncond);

/**
 * @brief Select a branch randomly.
 *
 * Selector function data is not used.
 */
ARCHI_HSP_BRANCH_SELECTOR_FUNC(archi_hsp_branch_select__random);

/**
 * @brief Simple loop - choose 0th branch N times, then choose 1st branch and reset the iteration counter to zero.
 *
 * Selector function data type: size_t[2].
 * The first size_t is the number of iterations to do,
 * the second size_t is the iteration counter.
 */
ARCHI_HSP_BRANCH_SELECTOR_FUNC(archi_hsp_branch_select__loop);

#endif // _ARCHI_HSP_HSP_BRANCH_STATE_FUN_H_

