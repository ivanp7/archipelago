/**
 * @file
 * @brief Application context interfaces for OpenCL event arrays.
 */

#pragma once
#ifndef _ARCHI_OPENCL_CTX_EVENT_VAR_H_
#define _ARCHI_OPENCL_CTX_EVENT_VAR_H_

#include "archi/context/api/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_event_array_init);   ///< OpenCL event array initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_event_array_final); ///< OpenCL event array finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_event_array_get);     ///< OpenCL event array getter function.

extern
const archi_context_interface_t archi_context_opencl_event_array_interface; ///< OpenCL event array interface.

#endif // _ARCHI_OPENCL_CTX_EVENT_VAR_H_

