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
 * @brief Types for OpenCL memory interfaces.
 */

#pragma once
#ifndef _ARCHI_OPENCL_MEM_SVM_TYP_H_
#define _ARCHI_OPENCL_MEM_SVM_TYP_H_

#include <CL/cl.h>


/**
 * @brief Memory allocation function data: OpenCL shared virtual memory.
 */
typedef struct archi_memory_alloc_data__opencl_svm {
    cl_context context; ///< OpenCL context.
    cl_svm_mem_flags mem_flags; ///< Shared virtual memory flags.
} archi_memory_alloc_data__opencl_svm_t;

/**
 * @brief Memory mapping function data: OpenCL shared virtual memory.
 */
typedef struct archi_memory_map_data__opencl_svm {
    cl_command_queue command_queue; ///< OpenCL command queue.
    cl_map_flags map_flags; ///< Memory mapping flags.
} archi_memory_map_data__opencl_svm_t;

#endif // _ARCHI_OPENCL_MEM_SVM_TYP_H_

