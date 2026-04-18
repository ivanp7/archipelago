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
 * @brief Data for DEG operation functions for OpenCL kernel operations.
 */

#pragma once
#ifndef _ARCHI_OPENCL_EXE_KERNEL_TYP_H_
#define _ARCHI_OPENCL_EXE_KERNEL_TYP_H_

#include "archi/opencl/api/event.typ.h"


/**
 * @brief Operation function data: enqueue a kernel execution.
 *
 * `work_offset_global` may be NULL, which is equivalent to array of zeros.
 * `work_size_local` may be NULL, which is equivalent to local work size picked automatically.
 */
typedef struct archi_dexgraph_op_data__opencl_kernel_enqueue {
    archi_opencl_event_array_t wait_list; ///< Events to wait for before enqueueing.
    archi_opencl_event_ptr_array_t out_list; ///< List of pointers to write output event to.

    cl_command_queue command_queue; ///< Command queue to enqueue the kernel to.
    cl_kernel kernel; ///< Kernel to execute.

    size_t num_work_dimensions; ///< Number of work dimensions.
    const size_t *work_offset_global; ///< Global work offset (or NULL).
    const size_t *work_size_global;   ///< Global work size.
    const size_t *work_size_local;    ///< Local work size (or NULL).
} archi_dexgraph_op_data__opencl_kernel_enqueue_t;

#endif // _ARCHI_OPENCL_EXE_KERNEL_TYP_H_

