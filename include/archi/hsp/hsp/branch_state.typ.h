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
 * @brief Types used in HSP branch states.
 */

#pragma once
#ifndef _ARCHI_HSP_HSP_BRANCH_STATE_TYP_H_
#define _ARCHI_HSP_HSP_BRANCH_STATE_TYP_H_

#include "archi/hsp/api/state.typ.h"

/**
 * @brief Declarator of a branch selector function.
 *
 * @return Index of the selected branch.
 */
#define ARCHI_HSP_BRANCH_SELECTOR_FUNC(name) size_t name(   \
        size_t num_branches, /* [in] Number of branches. */ \
        void *data) /* [in] Data for the function to operate on. */

/**
 * @brief Branch selector function.
 */
typedef ARCHI_HSP_BRANCH_SELECTOR_FUNC((*archi_hsp_branch_selector_func_t));

/**
 * @brief Function type tag for branch selector functions.
 */
#define ARCHI_POINTER_FUNCTION_TAG__HSP_BRANCH_SELECTOR     4

/*****************************************************************************/

/**
 * @brief Data for a branch state.
 */
typedef struct archi_hsp_state_data__branch {
    archi_hsp_branch_selector_func_t selector_fn; ///< Branch selector function.
    void *selector_data;                          ///< Branch selector data.

    const size_t num_branches;   ///< Number of branches.
    archi_hsp_frame_t *branch[]; ///< Array of branches.
} archi_hsp_state_data__branch_t;

#endif // _ARCHI_HSP_HSP_BRANCH_STATE_TYP_H_

