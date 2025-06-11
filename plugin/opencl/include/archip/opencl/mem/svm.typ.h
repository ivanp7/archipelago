/**
 * @file
 * @brief Types for OpenCL memory interface.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_MEM_SVM_TYP_H_
#define _ARCHIP_OPENCL_MEM_SVM_TYP_H_

#include <CL/cl.h>

/**
 * @brief Allocation parameters for archip_opencl_svm_alloc().
 */
typedef struct archip_opencl_svm_alloc_data {
    cl_context context; ///< OpenCL context.
    cl_svm_mem_flags mem_flags; ///< Shared virtual memory flags.
} archip_opencl_svm_alloc_data_t;

/**
 * @brief Mapping parameters for archip_opencl_svm_map().
 */
typedef struct archip_opencl_svm_map_data {
    cl_command_queue command_queue; ///< OpenCL command queue.
    cl_map_flags map_flags; ///< Memory mapping flags.
} archip_opencl_svm_map_data_t;

#endif // _ARCHIP_OPENCL_MEM_SVM_TYP_H_

