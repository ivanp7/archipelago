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
 * @brief Application context interface for HSP branch state data.
 */

#pragma once
#ifndef _ARCHI_HSP_CTX_STATE_BRANCH_DATA_VAR_H_
#define _ARCHI_HSP_CTX_STATE_BRANCH_DATA_VAR_H_

#include "archipelago/context/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_hsp_branch_state_data_init);   ///< HSP branch state data initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_hsp_branch_state_data_final); ///< HSP branch state data finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_hsp_branch_state_data_get);     ///< HSP branch state data field getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_hsp_branch_state_data_set);     ///< HSP branch state data field setter function.

extern
const archi_context_interface_t archi_context_hsp_branch_state_data_interface; ///< HSP branch state data interface.

#endif // _ARCHI_HSP_CTX_STATE_BRANCH_DATA_VAR_H_

