/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Operations on OpenCL devices.
 */

#pragma once
#ifndef _ARCHI_OPENCL_API_DEVICE_FUN_H_
#define _ARCHI_OPENCL_API_DEVICE_FUN_H_

#include "archi/opencl/api/device.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Allocate an object storing platform/device IDs.
 *
 * @note The returned pointer must be eventually released using free().
 *
 * @return Newly allocated object storing platform/device IDs.
 */
archi_opencl_platdev_t*
archi_opencl_platdev_alloc(
        cl_uint num_devices ///< [in] Length of OpenCL device ID array.
);

/**
 * @brief Set platform/device IDs by index.
 */
void
archi_opencl_platdev_set(
        archi_opencl_platdev_t *platdev, ///< [out] Platform/device IDs.

        cl_uint platform_index, ///< [in] Index of a platform.
        const cl_uint device_index[], ///< [in] Array of indices of devices of the platform.

        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_OPENCL_API_DEVICE_FUN_H_

