/**
 * @file
 * @brief Memory interface for OpenCL memory.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_MEM_SVM_VAR_H_
#define _ARCHIP_OPENCL_MEM_SVM_VAR_H_

#include "archi/mem/interface.typ.h"

/**
 * @brief OpenCL shared virtual memory allocation function.
 *
 * @p alloc_data must point to an object of type archip_opencl_svm_alloc_data_t.
 */
ARCHI_MEMORY_ALLOC_FUNC(archip_opencl_svm_alloc);

/**
 * @brief OpenCL shared virtual memory deallocation function.
 *
 * Frees a block of heap memory previously allocated by archip_opencl_svm_alloc().
 */
ARCHI_MEMORY_FREE_FUNC(archip_opencl_svm_free);

/**
 * @brief OpenCL shared virtual memory mapping function.
 */
ARCHI_MEMORY_MAP_FUNC(archip_opencl_svm_map);

/**
 * @brief OpenCL shared virtual memory unmapping function.
 */
ARCHI_MEMORY_UNMAP_FUNC(archip_opencl_svm_unmap);

/**
 * @brief OpenCL shared virtual memory interface.
 */
extern
const archi_memory_interface_t archip_opencl_svm_interface;

#endif // _ARCHIP_OPENCL_MEM_SVM_VAR_H_

