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
 * @brief Types for thread group dispatch HSP state.
 */

#pragma once
#ifndef _ARCHI_RES_THREAD_HSP_DISPATCH_TYP_H_
#define _ARCHI_RES_THREAD_HSP_DISPATCH_TYP_H_

#include "archi/res_thread/api/thread_group.fun.h"

/**
 * @brief Parameters for archi_thread_group_dispatch().
 */
typedef struct archi_thread_group_dispatch_data {
    archi_thread_group_context_t context; ///< Thread group context.

    archi_thread_group_work_t *work; ///< Concurrent work task.
    archi_thread_group_callback_t *callback; ///< Concurrent work completion callback.
    archi_thread_group_dispatch_params_t params; ///< Dispatch parameters.

    const char *name; ///< Name of the operation (for logging).
} archi_thread_group_dispatch_data_t;

#endif // _ARCHI_RES_THREAD_HSP_DISPATCH_TYP_H_

