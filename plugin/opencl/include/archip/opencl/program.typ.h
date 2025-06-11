/**
 * @file
 * @brief Types for collections of OpenCL program sources and binaries.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_PROGRAM_TYP_H_
#define _ARCHIP_OPENCL_PROGRAM_TYP_H_

#include "archip/opencl/device.typ.h"

#include <stddef.h> // for size_t

/**
 * @brief Collection of source files of an OpenCL program.
 */
typedef struct archip_opencl_program_sources {
    size_t num_files; ///< Number of files.
    char **pathnames; ///< Pathnames of files.

    size_t *sizes;    ///< Sizes of files.
    char **contents;  ///< Contents of files.
} archip_opencl_program_sources_t;

/**
 * @brief Collection of binaries of an OpenCL program.
 */
typedef struct archip_opencl_program_binaries {
    archip_opencl_platform_device_ids_t *ids; ///< IDs of the platform and devices the binaries are compiled for.

    size_t *sizes;            ///< Sizes of the binaries for each device.
    unsigned char **contents; ///< Binaries for each device.
} archip_opencl_program_binaries_t;

#endif // _ARCHIP_OPENCL_PROGRAM_TYP_H_

