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
 * @brief Data for DEG operation functions for thread group operations.
 */

#pragma once
#ifndef _ARCHI_THREAD_EXE_THREAD_GROUP_TYP_H_
#define _ARCHI_THREAD_EXE_THREAD_GROUP_TYP_H_

#include "archi/thread/api/handle.typ.h"
#include "archi/thread/api/work.typ.h"
#include "archi/thread/api/callback.typ.h"
#include "archi/thread/api/thread_group.typ.h"


/**
 * @brief Operation function data: dispatch work task to a thread group.
 */
typedef struct archi_dexgraph_op_data__thread_group_dispatch {
    archi_thread_group_t thread_group; ///< Thread group handle.

    archi_thread_group_work_t work; ///< Concurrent work task.
    archi_thread_group_callback_t callback; ///< Concurrent work completion callback.
    archi_thread_group_dispatch_params_t param; ///< Dispatch parameters.
} archi_dexgraph_op_data__thread_group_dispatch_t;

#endif // _ARCHI_THREAD_EXE_THREAD_GROUP_TYP_H_

