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
 * @brief Thread group callbacks for flag barriers.
 */

#pragma once
#ifndef _ARCHI_RES_THREAD_API_CALLBACK_FLAG_BARRIER_FUN_H_
#define _ARCHI_RES_THREAD_API_CALLBACK_FLAG_BARRIER_FUN_H_

#include "archi/res_thread/api/callback.typ.h"

/**
 * @brief Thread group callback for releasing flag barrier.
 *
 * This callback function expects archi_thread_flag_barrier_t
 * object as function data.
 */
ARCHI_THREAD_GROUP_CALLBACK_FUNC(archi_thread_group_callback_flag_barrier_release);

#endif // _ARCHI_RES_THREAD_API_CALLBACK_FLAG_BARRIER_FUN_H_

