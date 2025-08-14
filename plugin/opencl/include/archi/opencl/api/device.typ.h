/**
 * @file
 * @brief Types for operations on OpenCL devices.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_DEVICE_TYP_H_
#define _ARCHI_OPENCL_API_DEVICE_TYP_H_

#include <CL/cl.h>

/**
 * @brief Platform and device IDs needed to create a context.
 */
typedef struct archi_opencl_platform_device_ids {
    cl_platform_id platform_id; ///< OpenCL platform ID.

    const cl_uint num_devices; ///< Number of OpenCL device IDs in the array.
    cl_device_id device_id[];  ///< Array of OpenCL device IDs.
} archi_opencl_platform_device_ids_t;

#endif // _ARCHI_OPENCL_API_DEVICE_TYP_H_

