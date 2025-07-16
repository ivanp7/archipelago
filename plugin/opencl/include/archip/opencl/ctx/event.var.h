/**
 * @file
 * @brief Application context interfaces for OpenCL event arrays.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_CTX_EVENT_VAR_H_
#define _ARCHIP_OPENCL_CTX_EVENT_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_event_array_init);   ///< OpenCL event array initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_event_array_final); ///< OpenCL event array finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_event_array_get);     ///< OpenCL event array getter function.

extern
const archi_context_interface_t archip_context_opencl_event_array_interface; ///< OpenCL event array interface.

#endif // _ARCHIP_OPENCL_CTX_EVENT_VAR_H_

