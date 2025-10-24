/**
 * @file
 * @brief Application context interfaces for OpenCL command queues.
 */

#pragma once
#ifndef _ARCHI_OPENCL_CTX_COMMAND_QUEUE_VAR_H_
#define _ARCHI_OPENCL_CTX_COMMAND_QUEUE_VAR_H_

#include "archi/context/api/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_command_queue_init);   ///< OpenCL command queue initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_command_queue_final); ///< OpenCL command queue finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_command_queue_get);     ///< OpenCL command queue getter function.

extern
const archi_context_interface_t archi_context_opencl_command_queue_interface; ///< OpenCL command queue interface.

#endif // _ARCHI_OPENCL_CTX_COMMAND_QUEUE_VAR_H_

