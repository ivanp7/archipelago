/**
 * @file
 * @brief Operations on OpenCL devices.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_DEVICE_FUN_H_
#define _ARCHIP_OPENCL_DEVICE_FUN_H_

#include "archip/opencl/device.typ.h"
#include "archi/util/status.typ.h"

/**
 * @brief Obtain IDs of a platform and devices on that platform.
 *
 * @note The returned pointer must be eventually released using free().
 *
 * @return Structure storing IDs of the platform and devices.
 */
archip_opencl_platform_device_ids_t*
archip_opencl_get_platform_device_ids(
        cl_uint platform_index, ///< [in] Index of a platform.

        cl_uint num_device_indices,  ///< [in] Number of the device indices in the array.
        const cl_uint *device_index, ///< [in] Array of indices of devices of the platform.

        archi_status_t *code ///< [out] Status code.
);

#endif // _ARCHIP_OPENCL_DEVICE_FUN_H_

