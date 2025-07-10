/**
 * @file
 * @brief Functions for getting OpenCL status names from codes.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_STATUS_FUN_H_
#define _ARCHIP_OPENCL_STATUS_FUN_H_

#include <CL/cl.h>

/**
 * @brief Get OpenCL error name by value.
 *
 * @return OpenCL error name, or NULL for unknown code.
 */
const char*
archip_opencl_error_string(
        cl_int err ///< Error code returned by an OpenCL operation.
);

/**
 * @brief Get OpenCL program build status name by value.
 *
 * @return OpenCL build status name.
 */
const char*
archip_opencl_build_status_string(
        cl_build_status status ///< Program build status.
);

#endif // _ARCHIP_OPENCL_STATUS_FUN_H_

