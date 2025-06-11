/**
 * @file
 * @brief Application context interfaces for OpenCL kernel execution.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_CTX_EXEC_VAR_H_
#define _ARCHIP_OPENCL_CTX_EXEC_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_kernel_exec_data_init);   ///< OpenCL event initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_kernel_exec_data_final); ///< OpenCL event finalization function.

extern
const archi_context_interface_t archip_context_opencl_kernel_exec_data_interface; ///< OpenCL event interface.

#endif // _ARCHIP_OPENCL_CTX_EXEC_VAR_H_

