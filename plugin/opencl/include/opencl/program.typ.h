/**
 * @file
 * @brief Types for OpenCL program sources.
 */

#pragma once
#ifndef _PLUGIN_OPENCL_PROGRAM_TYP_H_
#define _PLUGIN_OPENCL_PROGRAM_TYP_H_

#include <CL/cl.h>
#include <stddef.h>

/**
 * @brief Set of header/source files of an OpenCL program.
 */
typedef struct plugin_opencl_program_files {
    size_t num_files; ///< Number of files.

    const char **pathnames; ///< Pathnames of files.
    const size_t *sizes;    ///< Sizes of files.
    const char **contents;  ///< Contents of files.
} plugin_opencl_program_files_t;

/**
 * @brief Set of binaries of an OpenCL program.
 */
typedef struct plugin_opencl_program_binaries {
    cl_platform_id platform_id;     ///< OpenCL platform the binaries are compiled for.
    cl_uint num_devices;            ///< Number of OpenCL devices the binaries are compiled for.
    const cl_device_id *device_ids; ///< Array of OpenCL device identifiers the binaries are compiled for.

    const size_t *sizes;            ///< Sizes of the binaries for each device.
    const unsigned char **contents; ///< Binaries for each device.
} plugin_opencl_program_binaries_t;

#endif // _PLUGIN_OPENCL_PROGRAM_TYP_H_

