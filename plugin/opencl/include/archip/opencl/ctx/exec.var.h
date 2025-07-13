/**
 * @file
 * @brief Application context interfaces for OpenCL kernel execution.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_CTX_EXEC_VAR_H_
#define _ARCHIP_OPENCL_CTX_EXEC_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_event_array_init);   ///< OpenCL event array initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_event_array_final); ///< OpenCL event array finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_event_array_get);     ///< OpenCL event array getter function.

extern
const archi_context_interface_t archip_context_opencl_event_array_interface; ///< OpenCL event array interface.

///////////////////////////////////////////////////////////////////////////////

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_work_vector_init);   ///< OpenCL work vector initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_work_vector_final); ///< OpenCL work vector finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_work_vector_get);     ///< OpenCL work vector getter function.
ARCHI_CONTEXT_SET_FUNC(archip_context_opencl_work_vector_set);     ///< OpenCL work vector setter function.

extern
const archi_context_interface_t archip_context_opencl_work_vector_interface; ///< OpenCL work vector interface.

///////////////////////////////////////////////////////////////////////////////

ARCHI_CONTEXT_INIT_FUNC(archip_opencl_kernel_enqueue_data_init);   ///< OpenCL kernel enqueue data initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_opencl_kernel_enqueue_data_final); ///< OpenCL kernel enqueue data finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_opencl_kernel_enqueue_data_get);     ///< OpenCL kernel enqueue data getter function.
ARCHI_CONTEXT_SET_FUNC(archip_opencl_kernel_enqueue_data_set);     ///< OpenCL kernel enqueue data setter function.

extern
const archi_context_interface_t archip_opencl_kernel_enqueue_data_interface; ///< OpenCL kernel enqueue data interface.

#endif // _ARCHIP_OPENCL_CTX_EXEC_VAR_H_

