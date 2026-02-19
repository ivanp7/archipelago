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
 * @brief Context interface for OpenCL kernels.
 */

#pragma once
#ifndef _ARCHI_OPENCL_CTX_KERNEL_VAR_H_
#define _ARCHI_OPENCL_CTX_KERNEL_VAR_H_

#include "archi/context/api/interface.typ.h"


/**
 * @brief Context interface: OpenCL kernel created anew.
 *
 * Initialization parameters:
 * - "program"  : (cl_program) OpenCL program
 * - "name"     : (char[]) kernel name
 *
 * Getter slots:
 * - "program"  : (cl_program) OpenCL program
 * - "name"     : (char[]) kernel name
 * - "num_args" : (cl_uint) number of kernel arguments
 *
 * Setter slots:
 * - "arg.local_mem_size" [index]   : (size_t) set local memory size for kernel argument #index
 * - "arg.value" [index]            : set kernel argument #index to value
 * - "arg.mem_object" [index]       : (cl_mem) set kernel argument #index to cl_mem object
 * - "arg.svm_ptr" [index]          : set kernel argument #index to SVM pointer
 * - "exec_info.svm_ptrs"           : (void*[]) array of non-argument SVM pointers used by kernel
 */
extern
const archi_context_interface_t
archi_context_interface__opencl_kernel_new;

/**
 * @brief Context interface: OpenCL kernel cloned from another kernel.
 *
 * Initialization parameters:
 * - "kernel"   : (cl_kernel) original OpenCL kernel
 *
 * Getter slots:
 * - "program"  : (cl_program) OpenCL program
 * - "name"     : (char[]) kernel name
 * - "num_args" : (cl_uint) number of kernel arguments
 *
 * Setter slots:
 * - "arg.local_mem_size" [index]   : (size_t) set local memory size for kernel argument #index
 * - "arg.value" [index]            : set kernel argument #index to value
 * - "arg.mem_object" [index]       : (cl_mem) set kernel argument #index to cl_mem object
 * - "arg.svm_ptr" [index]          : set kernel argument #index to SVM pointer
 * - "exec_info.svm_ptrs"           : (void*[]) array of non-argument SVM pointers used by kernel
 */
extern
const archi_context_interface_t
archi_context_interface__opencl_kernel_clone;

#endif // _ARCHI_OPENCL_CTX_KERNEL_VAR_H_

