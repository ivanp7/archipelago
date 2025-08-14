/**
 * @file
 * @brief OpenCL work description.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_WORK_TYP_H_
#define _ARCHI_OPENCL_API_WORK_TYP_H_

#include <CL/cl.h>

/**
 * @brief OpenCL kernel work vector.
 */
typedef struct archi_opencl_work_vector {
    const cl_uint num_dimensions; ///< Number of vector dimensions.
    size_t dimension[]; ///< Array of vector components.
} archi_opencl_work_vector_t;

#endif // _ARCHI_OPENCL_API_WORK_TYP_H_

