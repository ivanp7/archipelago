/**
 * @file
 * @brief Memory operations.
 *
 * The functions used are: clSVMAlloc(), clSVMFree(), clEnqueueSVMMap(), clEnqueueSVMUnmap().
 */

#pragma once
#ifndef _PLUGIN_OPENCL_MEMORY_FUN_H_
#define _PLUGIN_OPENCL_MEMORY_FUN_H_

#include "archi/util/memory.typ.h"

ARCHI_MEMORY_ALLOC_FUNC(plugin_opencl_memory_svm_alloc_func); ///< Shared virtual memory allocation function.
ARCHI_MEMORY_FREE_FUNC(plugin_opencl_memory_svm_free_func);   ///< Shared virtual memory deallocation function.
ARCHI_MEMORY_MAP_FUNC(plugin_opencl_memory_svm_map_func);     ///< Shared virtual memory mapping function.
ARCHI_MEMORY_UNMAP_FUNC(plugin_opencl_memory_svm_unmap_func); ///< Shared virtual memory unmapping function.

extern
const archi_memory_interface_t plugin_opencl_memory_svm_interface; ///< Shared virtual memory interface.

#endif // _PLUGIN_OPENCL_MEMORY_FUN_H_

