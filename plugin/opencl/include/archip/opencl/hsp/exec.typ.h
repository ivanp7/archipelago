/**
 * @file
 * @brief Types for hierarchical state processor states for OpenCL kernel execution.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_HSP_EXEC_TYP_H_
#define _ARCHIP_OPENCL_HSP_EXEC_TYP_H_

#include <CL/cl.h>

/**
 * @brief Array of OpenCL events.
 */
typedef struct archip_opencl_event_array {
    cl_event *event;    ///< Array of events.
    cl_uint num_events; ///< Number of events in the array.
} archip_opencl_event_array_t;

/**
 * @brief OpenCL kernel work description.
 */
typedef struct archip_opencl_work {
    size_t* global_work_offset; ///< Global work offset.
    size_t* global_work_size;   ///< Global work size.
    size_t* local_work_size;    ///< Local work size.

    cl_uint num_dimensions; ///< Number of work dimensions.
} archip_opencl_work_t;

/**
 * @brief Parameters for clEnqueueNDRangeKernel().
 */
typedef struct archip_opencl_kernel_enqueue_data {
    cl_command_queue command_queue; ///< Command queue to enqueue the kernel to.
    cl_kernel kernel; ///< Kernel to execute.
    archip_opencl_work_t *work; ///< Description of the work to do.

    archip_opencl_event_array_t *wait_list; ///< List of events to wait before execution.

    size_t num_event_copies; ///< Number of execution completion event copies required.
    archip_opencl_event_array_t **target_event_arrays; ///< Event arrays to copy the event to.
    cl_uint *target_event_array_indices; ///< Indices at target event arrays.

    const char *name; ///< Name of the operation (for logging).
} archip_opencl_kernel_enqueue_data_t;

#endif // _ARCHIP_OPENCL_HSP_EXEC_TYP_H_

