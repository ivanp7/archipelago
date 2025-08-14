/**
 * @file
 * @brief Application context interfaces for OpenCL shared virtual memory.
 */

#pragma once
#ifndef _ARCHI_OPENCL_CTX_SVM_VAR_H_
#define _ARCHI_OPENCL_CTX_SVM_VAR_H_

#include "archipelago/context/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_svm_alloc_data_init);   ///< SVM allocation parameters initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_svm_alloc_data_final); ///< SVM allocation parameters finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_svm_alloc_data_get);     ///< SVM allocation parameters getter function.

extern
const archi_context_interface_t archi_context_opencl_svm_alloc_data_interface; ///< SVM allocation parameters interface.

///////////////////////////////////////////////////////////////////////////////

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_svm_map_data_init);   ///< SVM mapping parameters initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_svm_map_data_final); ///< SVM mapping parameters finalization function.
ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_svm_map_data_get);     ///< SVM mapping parameters getter function.

extern
const archi_context_interface_t archi_context_opencl_svm_map_data_interface; ///< SVM mapping parameters interface.

#endif // _ARCHI_OPENCL_CTX_SVM_VAR_H_

