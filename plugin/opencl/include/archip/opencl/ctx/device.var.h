/**
 * @file
 * @brief Application context interfaces for OpenCL devices.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_CTX_DEVICE_VAR_H_
#define _ARCHIP_OPENCL_CTX_DEVICE_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_context_init);   ///< OpenCL context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_context_final); ///< OpenCL context finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_context_get);     ///< OpenCL context getter function.

extern
const archi_context_interface_t archip_context_opencl_context_interface; ///< OpenCL context interface.

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_command_queue_init);   ///< OpenCL command queue initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_command_queue_final); ///< OpenCL command queue finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_command_queue_get);     ///< OpenCL command queue getter function.

extern
const archi_context_interface_t archip_context_opencl_command_queue_interface; ///< OpenCL command queue interface.

#endif // _ARCHIP_OPENCL_CTX_DEVICE_VAR_H_

