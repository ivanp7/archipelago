/**
 * @file
 * @brief Application context interface for data of HSP state for OpenCL kernel execution.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_CTX_EXEC_VAR_H_
#define _ARCHIP_OPENCL_CTX_EXEC_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_opencl_kernel_enqueue_data_init);   ///< OpenCL kernel enqueue data initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_opencl_kernel_enqueue_data_final); ///< OpenCL kernel enqueue data finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_opencl_kernel_enqueue_data_get);     ///< OpenCL kernel enqueue data getter function.
ARCHI_CONTEXT_SET_FUNC(archip_opencl_kernel_enqueue_data_set);     ///< OpenCL kernel enqueue data setter function.

extern
const archi_context_interface_t archip_opencl_kernel_enqueue_data_interface; ///< OpenCL kernel enqueue data interface.

#endif // _ARCHIP_OPENCL_CTX_EXEC_VAR_H_

