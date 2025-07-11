/**
 * @file
 * @brief Types for operations on OpenCL devices.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_DEVICE_TYP_H_
#define _ARCHIP_OPENCL_DEVICE_TYP_H_

#include <CL/cl.h>

/**
 * @brief Platform and device IDs needed to create a context.
 */
typedef struct archip_opencl_platform_device_ids {
    cl_platform_id platform_id; ///< OpenCL platform ID.

    const cl_uint num_devices; ///< Number of OpenCL device IDs in the array.
    cl_device_id device_id[];  ///< Array of OpenCL device IDs.
} archip_opencl_platform_device_ids_t;

#endif // _ARCHIP_OPENCL_DEVICE_TYP_H_

