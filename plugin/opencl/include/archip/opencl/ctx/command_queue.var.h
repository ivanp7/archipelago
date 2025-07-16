/**
 * @file
 * @brief Application context interfaces for OpenCL command queues.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_CTX_COMMAND_QUEUE_VAR_H_
#define _ARCHIP_OPENCL_CTX_COMMAND_QUEUE_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_command_queue_init);   ///< OpenCL command queue initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_command_queue_final); ///< OpenCL command queue finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_command_queue_get);     ///< OpenCL command queue getter function.

extern
const archi_context_interface_t archip_context_opencl_command_queue_interface; ///< OpenCL command queue interface.

#endif // _ARCHIP_OPENCL_CTX_COMMAND_QUEUE_VAR_H_

