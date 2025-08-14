/**
 * @file
 * @brief Hierarchical state processor states for OpenCL kernel execution.
 */

#pragma once
#ifndef _ARCHI_OPENCL_HSP_EXEC_FUN_H_
#define _ARCHI_OPENCL_HSP_EXEC_FUN_H_

#include "archi/hsp/api/state.typ.h"

/**
 * @brief State function for calling clWaitForEvents().
 *
 * This state function expects archi_opencl_event_array_t
 * object as function data.
 */
ARCHI_HSP_STATE_FUNCTION(archi_opencl_hsp_state_wait_for_events);

/**
 * @brief State function for calling clEnqueueNDRangeKernel().
 *
 * This state function expects archi_opencl_kernel_enqueue_data_t
 * object as function data.
 */
ARCHI_HSP_STATE_FUNCTION(archi_opencl_hsp_state_kernel_enqueue);

#endif // _ARCHI_OPENCL_HSP_EXEC_FUN_H_

