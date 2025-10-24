/**
 * @file
 * @brief Application context interfaces for OpenCL work description.
 */

#pragma once
#ifndef _ARCHI_OPENCL_CTX_WORK_VAR_H_
#define _ARCHI_OPENCL_CTX_WORK_VAR_H_

#include "archi/context/api/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_work_vector_init);   ///< OpenCL work vector initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_work_vector_final); ///< OpenCL work vector finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_work_vector_get);     ///< OpenCL work vector getter function.
ARCHI_CONTEXT_SET_FUNC(archi_context_opencl_work_vector_set);     ///< OpenCL work vector setter function.

extern
const archi_context_interface_t archi_context_opencl_work_vector_interface; ///< OpenCL work vector interface.

#endif // _ARCHI_OPENCL_CTX_WORK_VAR_H_

