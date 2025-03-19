/**
 * @file
 * @brief Building OpenCL programs from sources.
 */

#pragma once
#ifndef _PLUGIN_OPENCL_PROGRAM_FUN_H_
#define _PLUGIN_OPENCL_PROGRAM_FUN_H_

#include "opencl/program.typ.h"
#include "archi/util/status.typ.h"

#include <stdbool.h>

/**
 * @brief Concatenate compiler/linker flags.
 *
 * @return String of OpenCL compiler/linker flags.
 */
const char*
plugin_opencl_program_concat_flags(
        const char *flags[] ///< [in] Array of strings to concatenate (NULL-terminated).
);

/**
 * @brief Build an OpenCL program from sources.
 *
 * @return OpenCL program object.
 */
cl_program
plugin_opencl_program_build(
        cl_context context,  ///< [in] Valid OpenCL context.
        cl_uint num_devices, ///< [in] Number of devices listed in the device list.
        const cl_device_id *device_list, ///< [in] List of devices to build the program for.

        cl_uint num_header_sets, ///< [in] Number of sets of header files.
        const plugin_opencl_program_files_t *header_sets[], ///< [in] Array of sets of header files.

        cl_uint num_source_sets, ///< [in] Number of sets of source files.
        const plugin_opencl_program_files_t *source_sets[], ///< [in] Array of sets of source sets.

        cl_uint num_libraries,        ///< [in] Number of libraries to link.
        const cl_program libraries[], ///< [in] Array of libraries to link.

        const char *cflags, ///< [in] Compiler flags.
        const char *lflags, ///< [in] Linker flags.

        bool logging, ///< [in] Enable progress logging.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Create an OpenCL program from binaries.
 *
 * @return OpenCL program object.
 */
cl_program
plugin_opencl_program_create(
        cl_context context, ///< [in] Valid OpenCL context.

        plugin_opencl_program_binaries_t binaries, ///< [in] Binaries of an OpenCL program.

        bool logging, ///< [in] Enable progress logging.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Extract binaries an OpenCL program.
 *
 * @return Status code.
 */
archi_status_t
plugin_opencl_program_binaries_extract(
        cl_program program, ///< [in] OpenCL program.
        plugin_opencl_program_binaries_t *binaries ///< [out] Binaries of an OpenCL program.
);

/**
 * @brief Deallocate binaries an OpenCL program.
 */
void
plugin_opencl_program_binaries_free(
        plugin_opencl_program_binaries_t *binaries ///< [in] Binaries of an OpenCL program.
);

#endif // _PLUGIN_OPENCL_PROGRAM_FUN_H_

