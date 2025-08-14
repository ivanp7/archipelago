/**
 * @file
 * @brief Types for OpenCL memory interface.
 */

#pragma once
#ifndef _ARCHI_OPENCL_MEM_SVM_TYP_H_
#define _ARCHI_OPENCL_MEM_SVM_TYP_H_

#include <CL/cl.h>

/**
 * @brief Allocation parameters for archi_opencl_svm_alloc().
 */
typedef struct archi_opencl_svm_alloc_data {
    cl_context context; ///< OpenCL context.
    cl_svm_mem_flags mem_flags; ///< Shared virtual memory flags.
} archi_opencl_svm_alloc_data_t;

/**
 * @brief Mapping parameters for archi_opencl_svm_map().
 */
typedef struct archi_opencl_svm_map_data {
    cl_command_queue command_queue; ///< OpenCL command queue.
    cl_map_flags map_flags; ///< Memory mapping flags.
} archi_opencl_svm_map_data_t;

#endif // _ARCHI_OPENCL_MEM_SVM_TYP_H_

