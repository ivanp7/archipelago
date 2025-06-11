/**
 * @file
 * @brief Application context interfaces for OpenCL memory.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_CTX_MEMORY_VAR_H_
#define _ARCHIP_OPENCL_CTX_MEMORY_VAR_H_

#include "archi/ctx/interface.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_svm_alloc_data_init);   ///< SVM allocation parameters initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_svm_alloc_data_final); ///< SVM allocation parameters finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_svm_alloc_data_get);     ///< SVM allocation parameters getter function.
ARCHI_CONTEXT_SET_FUNC(archip_context_opencl_svm_alloc_data_set);     ///< SVM allocation parameters setter function.

extern
const archi_context_interface_t archip_context_opencl_svm_alloc_data_interface; ///< SVM allocation parameters interface.

///////////////////////////////////////////////////////////////////////////////

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_svm_map_data_init);   ///< SVM mapping parameters initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_svm_map_data_final); ///< SVM mapping parameters finalization function.
ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_svm_map_data_get);     ///< SVM mapping parameters getter function.
ARCHI_CONTEXT_SET_FUNC(archip_context_opencl_svm_map_data_set);     ///< SVM mapping parameters setter function.

extern
const archi_context_interface_t archip_context_opencl_svm_map_data_interface; ///< SVM mapping parameters interface.

#endif // _ARCHIP_OPENCL_CTX_MEMORY_VAR_H_

