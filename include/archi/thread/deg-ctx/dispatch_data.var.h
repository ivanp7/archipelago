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
 * @brief Context interface for data of DEG node function for thread group dispatch operation.
 */

#pragma once
#ifndef _ARCHI_THREAD_DEG_CTX_DISPATCH_DATA_VAR_H_
#define _ARCHI_THREAD_DEG_CTX_DISPATCH_DATA_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: data of DEG node function for thread group dispatch operation.
 *
 * Initialization parameters:
 * - "thread_group"         : (archi_thread_group_t) thread group handle
 * - "work_function"        : (archi_thread_group_work_func_t) work function
 * - "work_data"            : work data
 * - "callback_function"    : (archi_thread_group_callback_func_t) callback function
 * - "callback_data"        : callback data
 * - "param_offset"         : (size_t) work offset - offset of work item indices
 * - "param_size"           : (size_t) work size - number of work items to be processed
 * - "param_batch_size"     : (size_t) batch size - number of work items done by a thread at once
 *
 * Getter parameters:
 * - "thread_group"         : (archi_thread_group_t) thread group handle
 * - "work.function"        : (archi_thread_group_work_func_t) work function
 * - "work.data"            : work data
 * - "callback.function"    : (archi_thread_group_callback_func_t) callback function
 * - "callback.data"        : callback data
 * - "param.offset"         : (size_t) work offset - offset of work item indices
 * - "param.size"           : (size_t) work size - number of work items to be processed
 * - "param.batch_size"     : (size_t) batch size - number of work items done by a thread at once
 *
 * Setter parameters:
 * - "thread_group"         : (archi_thread_group_t) thread group handle
 * - "work.function"        : (archi_thread_group_work_func_t) work function
 * - "work.data"            : work data
 * - "callback.function"    : (archi_thread_group_callback_func_t) callback function
 * - "callback.data"        : callback data
 * - "param.offset"         : (size_t) work offset - offset of work item indices
 * - "param.size"           : (size_t) work size - number of work items to be processed
 * - "param.batch_size"     : (size_t) batch size - number of work items done by a thread at once
 */
extern
const archi_context_interface_t
archi_context_interface__deg_node_data__thread_group_dispatch;

#endif // _ARCHI_THREAD_DEG_CTX_DISPATCH_DATA_VAR_H_

