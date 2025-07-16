/**
 * @file
 * @brief Application context interfaces for OpenCL kernels.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_CTX_KERNEL_VAR_H_
#define _ARCHIP_OPENCL_CTX_KERNEL_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_kernel_init_new);  ///< OpenCL kernel initialization function (new).
ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_kernel_init_copy); ///< OpenCL kernel initialization function (copy).
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_kernel_final);    ///< OpenCL kernel finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_kernel_get);        ///< OpenCL kernel getter function.
ARCHI_CONTEXT_SET_FUNC(archip_context_opencl_kernel_set);        ///< OpenCL kernel setter function.

extern
const archi_context_interface_t archip_context_opencl_kernel_new_interface;  ///< OpenCL kernel interface (newly created).
extern
const archi_context_interface_t archip_context_opencl_kernel_copy_interface; ///< OpenCL kernel interface (copied from existing).

#endif // _ARCHIP_OPENCL_CTX_KERNEL_VAR_H_

