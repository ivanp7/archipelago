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
 * @brief Arrays of hierarchical state processor transitions.
 */

#pragma once
#ifndef _ARCHI_HSP_HSP_ARRAY_TRANSITION_FUN_H_
#define _ARCHI_HSP_HSP_ARRAY_TRANSITION_FUN_H_

#include "archi/hsp/hsp/array_transition.typ.h"

/**
 * @brief Allocate array transition data.
 *
 * @note The returned pointer must be eventually released using free().
 *
 * @return Newly allocated array transition data.
 */
archi_hsp_transition_data__array_t*
archi_hsp_transition_data_alloc__array(
        size_t num_transitions ///< [in] Number of branches.
);

/*****************************************************************************/

/**
 * @brief Support for arrays of transitions.
 *
 * Transition function data type: archi_hsp_transition_data__array_t.
 * State metadata is not used.
 */
ARCHI_HSP_TRANSITION_FUNCTION(archi_hsp_transition__array);

#endif // _ARCHI_HSP_HSP_ARRAY_TRANSITION_FUN_H_

