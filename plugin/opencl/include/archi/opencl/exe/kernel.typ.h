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

#include <CL/cl.h>


/**
 * @brief Operation function data: enqueue a kernel execution.
 *
 * `work_offset_global` may be NULL, which is equivalent to array of zeros.
 * `work_size_local` may be NULL, which is equivalent to local work size picked automatically.
 */
typedef struct archi_dexgraph_op_data__opencl_kernel_enqueue {
    cl_kernel kernel; ///< Kernel to execute.
    cl_command_queue command_queue; ///< Command queue to enqueue the kernel to.

    size_t num_work_dimensions; ///< Number of work dimensions.
    const size_t *work_offset_global; ///< Global work offset (or NULL).
    const size_t *work_size_global;   ///< Global work size.
    const size_t *work_size_local;    ///< Local work size (or NULL).

    size_t wait_list_length; ///< Number of events to wait for before enqueueing.
    cl_event *wait_list;     ///< Array of events to wait for before enqueueing.

    size_t num_out_event_ptrs; ///< Number of locations to write output event to.
    cl_event **out_event_ptr;  ///< Array of pointers to locations to write output event to.
} archi_dexgraph_op_data__opencl_kernel_enqueue_t;

#endif // _ARCHI_OPENCL_EXE_KERNEL_TYP_H_

