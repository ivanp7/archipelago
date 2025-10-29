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
 * @brief Context interface for lock-free queues.
 */

#pragma once
#ifndef _ARCHI_THREAD_CTX_LFQUEUE_VAR_H_
#define _ARCHI_THREAD_CTX_LFQUEUE_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Context interface: lock-free queue.
 *
 * Initialization parameters:
 * - "params"               : (archi_thread_lfqueue_alloc_params_t) queue creation parameters structure
 * - "capacity"             : (size_t) queue capacity
 * - "element_size"         : (size_t) queue element size in bytes
 * - "element_alignment"    : (size_t) queue element alignment requirement in bytes
 *
 * Getter slots:
 * - "capacity"             : (size_t) queue capacity
 * - "element.size"         : (size_t) queue element size in bytes
 * - "element.alignment"    : (size_t) queue element alignment requirement in bytes
 */
extern
const archi_context_interface_t
archi_context_interface__thread_lfqueue;

#endif // _ARCHI_THREAD_CTX_LFQUEUE_VAR_H_

