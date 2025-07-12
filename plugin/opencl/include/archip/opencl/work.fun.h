/**
 * @file
 * @brief Operations on OpenCL work description.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_WORK_FUN_H_
#define _ARCHIP_OPENCL_WORK_FUN_H_

#include "archip/opencl/work.typ.h"

/**
 * @brief Allocate an work vector.
 *
 * @note The returned pointer must be eventually released using free().
 *
 * @return Newly allocated event array.
 */
archip_opencl_work_vector_t*
archip_opencl_work_vector_alloc(
        cl_uint num_dimensions ///< [in] Number of dimension in the vector.
);

#endif // _ARCHIP_OPENCL_WORK_FUN_H_

