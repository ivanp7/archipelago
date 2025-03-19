/**
 * @file
 * @brief Types for memory operations.
 */

#pragma once
#ifndef _PLUGIN_OPENCL_MEMORY_TYP_H_
#define _PLUGIN_OPENCL_MEMORY_TYP_H_

#include "archi/util/memory.typ.h"

#include <CL/cl.h>

/**
 * @brief Parameters for archi_memory_alloc_func_t.
 */
typedef struct plugin_opencl_memory_alloc_config {
    archi_memory_alloc_config_t base; ///< Base structure.

    cl_context context; ///< OpenCL context.
    cl_svm_mem_flags svm_mem_flags; ///< Shared virtual memory flags.
} plugin_opencl_memory_alloc_config_t;

/**
 * @brief Parameters for archi_memory_map_func_t.
 */
typedef struct plugin_opencl_memory_map_config {
    archi_memory_map_config_t base; ///< Base structure.

    cl_command_queue command_queue; ///< OpenCL command queue.
    cl_map_flags map_flags; ///< Memory mapping flags.
} plugin_opencl_memory_map_config_t;

#endif // _PLUGIN_OPENCL_MEMORY_TYP_H_

