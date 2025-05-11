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
 * @brief Application context interface for thread groups.
 */

#pragma once
#ifndef _ARCHI_BUILTIN_RES_THREAD_GROUP_CONTEXT_VAR_H_
#define _ARCHI_BUILTIN_RES_THREAD_GROUP_CONTEXT_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_init);   ///< Thread group context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_final); ///< Thread group context finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_get);     ///< Thread group context getter function.

extern
const archi_context_interface_t archi_context_res_thread_group_interface; ///< Thread group context interface.

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_work_init);   ///< Thread group work initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_work_final); ///< Thread group work finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_work_get);     ///< Thread group work getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_res_thread_group_work_set);     ///< Thread group work setter function.

extern
const archi_context_interface_t archi_context_res_thread_group_work_interface; ///< Thread group work interface.

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_callback_init);   ///< Thread group callback initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_callback_final); ///< Thread group callback finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_callback_get);     ///< Thread group callback getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_res_thread_group_callback_set);     ///< Thread group callback setter function.

extern
const archi_context_interface_t archi_context_res_thread_group_callback_interface; ///< Thread group callback interface.

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archi_context_res_thread_group_dispatch_data_init);   ///< Thread group dispatch data initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_res_thread_group_dispatch_data_final); ///< Thread group dispatch data finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_res_thread_group_dispatch_data_get);     ///< Thread group dispatch data getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_res_thread_group_dispatch_data_set);     ///< Thread group dispatch data setter function.

extern
const archi_context_interface_t archi_context_res_thread_group_dispatch_data_interface; ///< Thread group dispatch data interface.

#endif // _ARCHI_BUILTIN_RES_THREAD_GROUP_CONTEXT_VAR_H_

