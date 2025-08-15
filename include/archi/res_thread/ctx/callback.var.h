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
 * @brief Application context interface for thread group callbacks.
 */

#pragma once
#ifndef _ARCHI_RES_THREAD_CTX_CALLBACK_VAR_H_
#define _ARCHI_RES_THREAD_CTX_CALLBACK_VAR_H_

#include "archipelago/context/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_thread_group_callback_init);   ///< Thread group callback initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_thread_group_callback_final); ///< Thread group callback finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_thread_group_callback_get);     ///< Thread group callback getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_thread_group_callback_set);     ///< Thread group callback setter function.

extern
const archi_context_interface_t archi_context_thread_group_callback_interface; ///< Thread group callback interface.

#endif // _ARCHI_RES_THREAD_CTX_CALLBACK_VAR_H_

