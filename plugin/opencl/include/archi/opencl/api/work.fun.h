/**
 * @file
 * @brief Operations on OpenCL work description.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_WORK_FUN_H_
#define _ARCHI_OPENCL_API_WORK_FUN_H_

#include "archi/opencl/api/work.typ.h"

/**
 * @brief Allocate an work vector.
 *
 * @note The returned pointer must be eventually released using free().
 *
 * @return Newly allocated event array.
 */
archi_opencl_work_vector_t*
archi_opencl_work_vector_alloc(
        cl_uint num_dimensions ///< [in] Number of dimension in the vector.
);

#endif // _ARCHI_OPENCL_API_WORK_FUN_H_

