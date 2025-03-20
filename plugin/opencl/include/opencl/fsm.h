/**
 * @file
 * @brief FSM states for OpenCL processing.
 */

#pragma once
#ifndef _PLUGIN_OPENCL_FSM_H_
#define _PLUGIN_OPENCL_FSM_H_

#include "archi/fsm/state.typ.h"

#include <CL/cl.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Enqueue an OpenCL kernel for execution.
 */
typedef struct plugin_opencl_fsm_state_data_enqueue_kernel {
    cl_command_queue command_queue; ///< OpenCL command queue.
    cl_kernel kernel; ///< OpenCL kernel.

    cl_uint work_dim; ///< Number of work dimensions.
    const size_t *global_work_offset; ///< Global work offset.
    const size_t *global_work_size;   ///< Global work size.
    const size_t *local_work_size;    ///< Local work size.

    cl_uint num_events_in_wait_list; ///< Number of events to wait for.
    const cl_event *event_wait_list; ///< Array of events to wait for.

    cl_event *event; ///< Event of the command.
    bool wait_for_completion; ///< Whether to wait for the command to complete.

    bool logging; ///< Whether error logging is enabled.
} plugin_opencl_fsm_state_data_enqueue_kernel_t;

/**
 * @brief Enqueue an OpenCL kernel for execution.
 */
ARCHI_FSM_STATE_FUNCTION(plugin_opencl_fsm_state_enqueue_kernel);

#endif // _PLUGIN_OPENCL_FSM_H_

