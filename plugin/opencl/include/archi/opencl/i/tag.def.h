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
 * @brief Data and function type tags.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_TAG_DEF_H_
#define _ARCHI_OPENCL_API_TAG_DEF_H_

#define ARCHI_POINTER_DATA_TAG__OPENCL_PLATFORM_ID      0x1000 ///< Data type tag for cl_platform_id.
#define ARCHI_POINTER_DATA_TAG__OPENCL_DEVICE_ID        0x1001 ///< Data type tag for cl_device_id.
#define ARCHI_POINTER_DATA_TAG__OPENCL_CONTEXT          0x1002 ///< Data type tag for cl_context.
#define ARCHI_POINTER_DATA_TAG__OPENCL_COMMAND_QUEUE    0x1003 ///< Data type tag for cl_command_queue.
#define ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM          0x1004 ///< Data type tag for cl_program.
#define ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL           0x1005 ///< Data type tag for cl_kernel.
#define ARCHI_POINTER_DATA_TAG__OPENCL_EVENT            0x1006 ///< Data type tag for cl_event.
#define ARCHI_POINTER_DATA_TAG__OPENCL_MEM_OBJECT       0x1007 ///< Data type tag for cl_mem.
#define ARCHI_POINTER_DATA_TAG__OPENCL_SVM              0x1008 ///< Data type tag for SVM memory allocation.
#define ARCHI_POINTER_DATA_TAG__OPENCL_SVM_ALLOC_DATA   0x1009 ///< Data type tag for archi_memory_alloc_data__opencl_svm_t.
#define ARCHI_POINTER_DATA_TAG__OPENCL_SVM_MAP_DATA     0x100A ///< Data type tag for archi_memory_map_data__opencl_svm_t.

#endif // _ARCHI_OPENCL_API_TAG_DEF_H_

