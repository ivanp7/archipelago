/**
 * @file
 * @brief Types for hierarchical state processor states for OpenCL kernel execution.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_HSP_EXEC_TYP_H_
#define _ARCHIP_OPENCL_HSP_EXEC_TYP_H_

#include "archip/opencl/work.typ.h"
#include "archip/opencl/event.typ.h"

/**
 * @brief Parameters for clEnqueueNDRangeKernel().
 */
typedef struct archip_opencl_kernel_enqueue_data {
    cl_command_queue command_queue; ///< Command queue to enqueue the kernel to.
    cl_kernel kernel; ///< Kernel to execute.

    archip_opencl_work_vector_t *global_work_offset; ///< Global work offset (or NULL).
    archip_opencl_work_vector_t *global_work_size;   ///< Global work size.
    archip_opencl_work_vector_t *local_work_size;    ///< Local work size (or NULL).

    archip_opencl_event_array_t *wait_list; ///< List of events to wait before execution.
    archip_opencl_event_ptr_list_t *event_target_list; ///< List of pointers to assign the event to.

    const char *name; ///< Name of the operation (for logging).
} archip_opencl_kernel_enqueue_data_t;

#endif // _ARCHIP_OPENCL_HSP_EXEC_TYP_H_

