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
 * @brief Context interface for OpenCL command queues.
 */

#pragma once
#ifndef _ARCHI_OPENCL_CTX_COMMAND_QUEUE_VAR_H_
#define _ARCHI_OPENCL_CTX_COMMAND_QUEUE_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: OpenCL command queue.
 *
 * Initialization parameters:
 * - "context"              : (cl_context) OpenCL context
 * - "device_id"            : (cl_device_id) OpenCL device ID
 * - "out_of_order_exec"    : (char) enable out-of-order execution
 * - "profiling"            : (char) enable profiling
 * - "priority_hint"        : (cl_queue_priority_khr) queue priority hint
 * - "throttle_hint"        : (cl_queue_throttle_khr) queue throttle hint
 *
 * Getter slots:
 * - "context"      : (cl_context) OpenCL context
 * - "platform_id"  : (cl_platform_id) OpenCL platform ID
 * - "device_id"    : (cl_device_id) OpenCL device ID
 */
extern
const archi_context_interface_t
archi_context_interface__opencl_command_queue;

#endif // _ARCHI_OPENCL_CTX_COMMAND_QUEUE_VAR_H_

