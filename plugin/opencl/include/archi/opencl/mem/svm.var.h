/**
 * @file
 * @brief Memory interface for OpenCL memory.
 */

#pragma once
#ifndef _ARCHI_OPENCL_MEM_SVM_VAR_H_
#define _ARCHI_OPENCL_MEM_SVM_VAR_H_

#include "archi/memory/api/interface.typ.h"

/**
 * @brief OpenCL shared virtual memory allocation function.
 *
 * @p alloc_data must point to an object of type archi_opencl_svm_alloc_data_t.
 */
ARCHI_MEMORY_ALLOC_FUNC(archi_opencl_svm_alloc);

/**
 * @brief OpenCL shared virtual memory deallocation function.
 *
 * Frees a block of heap memory previously allocated by archi_opencl_svm_alloc().
 */
ARCHI_MEMORY_FREE_FUNC(archi_opencl_svm_free);

/**
 * @brief OpenCL shared virtual memory mapping function.
 */
ARCHI_MEMORY_MAP_FUNC(archi_opencl_svm_map);

/**
 * @brief OpenCL shared virtual memory unmapping function.
 */
ARCHI_MEMORY_UNMAP_FUNC(archi_opencl_svm_unmap);

/**
 * @brief OpenCL shared virtual memory interface.
 */
extern
const archi_memory_interface_t archi_opencl_svm_interface;

#endif // _ARCHI_OPENCL_MEM_SVM_VAR_H_

