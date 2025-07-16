/**
 * @file
 * @brief Application context interfaces for OpenCL work description.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_CTX_WORK_VAR_H_
#define _ARCHIP_OPENCL_CTX_WORK_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_work_vector_init);   ///< OpenCL work vector initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_work_vector_final); ///< OpenCL work vector finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_work_vector_get);     ///< OpenCL work vector getter function.
ARCHI_CONTEXT_SET_FUNC(archip_context_opencl_work_vector_set);     ///< OpenCL work vector setter function.

extern
const archi_context_interface_t archip_context_opencl_work_vector_interface; ///< OpenCL work vector interface.

#endif // _ARCHIP_OPENCL_CTX_WORK_VAR_H_

