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
 * @brief Types forierarchical state processor states for execution branching.
 */

#pragma once
#ifndef _ARCHI_HSP_STATE_BRANCH_TYP_H_
#define _ARCHI_HSP_STATE_BRANCH_TYP_H_

#include "archi/hsp/state.typ.h"

/**
 * @brief Declarator of a branch selector function.
 *
 * @return Index of the selected branch.
 */
#define ARCHI_HSP_BRANCH_SELECTOR_FUNC(name) size_t name( \
        size_t num_branches, /* Number of branches. */ \
        void *const data) /* Data for the function to operate on. */

/**
 * @brief Selector function.
 */
typedef ARCHI_HSP_BRANCH_SELECTOR_FUNC((*archi_hsp_branch_selector_func_t));

/*****************************************************************************/

/**
 * @brief Data for a branch state.
 */
typedef struct archi_hsp_branch_state_data {
    archi_hsp_branch_selector_func_t selector_fn; ///< Branch selector function.
    void *selector_data;                          ///< Branch selector data.

    const size_t num_branches;   ///< Number of branches.
    archi_hsp_frame_t *branch[]; ///< Array of branches.
} archi_hsp_branch_state_data_t;

/*****************************************************************************/

/**
 * @brief Data for the simple loop selector function.
 */
typedef struct archi_hsp_branch_select_loop_data {
    size_t num_iterations; ///< Number of iterations to do.
    size_t iteration; ///< Number of the current iteration.
} archi_hsp_branch_select_loop_data_t;

#endif // _ARCHI_HSP_STATE_BRANCH_TYP_H_

