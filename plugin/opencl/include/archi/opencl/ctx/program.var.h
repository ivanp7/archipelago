/**
 * @file
 * @brief Application context interfaces for OpenCL programs.
 */

#pragma once
#ifndef _ARCHI_OPENCL_CTX_PROGRAM_VAR_H_
#define _ARCHI_OPENCL_CTX_PROGRAM_VAR_H_

#include "archi/context/api/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_program_init_src); ///< OpenCL program initialization function (from sources).
ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_program_init_bin); ///< OpenCL program initialization function (from binaries).
ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_program_final);   ///< OpenCL program finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_program_get);       ///< OpenCL program getter function.

extern
const archi_context_interface_t archi_context_opencl_program_src_interface; ///< OpenCL program interface (initialization from sources).
extern
const archi_context_interface_t archi_context_opencl_program_bin_interface; ///< OpenCL program interface (initialization from binaries).


#endif // _ARCHI_OPENCL_CTX_PROGRAM_VAR_H_

