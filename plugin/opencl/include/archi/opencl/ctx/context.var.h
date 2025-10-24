/**
 * @file
 * @brief Application context interfaces for OpenCL contexts.
 */

#pragma once
#ifndef _ARCHI_OPENCL_CTX_CONTEXT_VAR_H_
#define _ARCHI_OPENCL_CTX_CONTEXT_VAR_H_

#include "archi/context/api/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_context_init);   ///< OpenCL context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_context_final); ///< OpenCL context finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_context_get);     ///< OpenCL context getter function.

extern
const archi_context_interface_t archi_context_opencl_context_interface; ///< OpenCL context interface.

#endif // _ARCHI_OPENCL_CTX_CONTEXT_VAR_H_

