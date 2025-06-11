/**
 * @file
 * @brief Operations on OpenCL programs.
 */

#pragma once
#ifndef _ARCHIP_OPENCL_PROGRAM_FUN_H_
#define _ARCHIP_OPENCL_PROGRAM_FUN_H_

#include "archip/opencl/program.typ.h"
#include "archi/util/pointer.typ.h"
#include "archi/util/status.typ.h"

#include <stdbool.h>

/**
 * @brief Concatenate compiler/linker flags.
 *
 * @return String of OpenCL compiler/linker flags.
 */
const char*
archip_opencl_program_concat_flags(
        const char *flags[] ///< [in] Array of strings to concatenate (NULL-terminated).
);

/**
 * @brief Create a copy of program sources from a hashmap.
 *
 * Hashmap keys are expected to be pathnames of the corresponding files.
 *
 * @return OpenCL program sources.
 */
archip_opencl_program_sources_t
archip_opencl_program_sources_from_hashmap(
        void *hashmap, ///< [in] Hashmap.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Deallocate sources an OpenCL program.
 */
void
archip_opencl_program_sources_free(
        archip_opencl_program_sources_t sources ///< [in] Sources of an OpenCL program.
);

/**
 * @brief Create a copy of program binaries from an array of pointers.
 *
 * @return OpenCL program binaries.
 */
archip_opencl_program_binaries_t
archip_opencl_program_binaries_from_array(
        archi_pointer_t *array, ///< [in] Array of pointers.
        size_t num_elements, ///< [in] Number of elements in the array.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Deallocate binaries an OpenCL program.
 */
void
archip_opencl_program_binaries_free(
        archip_opencl_program_binaries_t binaries ///< [in] Binaries of an OpenCL program.
);

/**
 * @brief Build an OpenCL program from sources.
 *
 * @return OpenCL program object.
 */
cl_program
archip_opencl_program_build(
        cl_context context,  ///< [in] Valid OpenCL context.
        cl_uint num_devices, ///< [in] Number of devices listed in the device list.
        const cl_device_id device_id[], ///< [in] List of devices to build the program for.

        archip_opencl_program_sources_t headers, ///< [in] Program headers.
        archip_opencl_program_sources_t sources, ///< [in] Program sources.

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
archip_opencl_program_create(
        cl_context context, ///< [in] Valid OpenCL context.

        archip_opencl_program_binaries_t binaries, ///< [in] Binaries of an OpenCL program.

        bool logging, ///< [in] Enable progress logging.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Extract binaries an OpenCL program.
 *
 * @return Binaries of an OpenCL program.
 */
archip_opencl_program_binaries_t
archip_opencl_program_binaries_extract(
        cl_program program, ///< [in] OpenCL program.
        archi_status_t *code ///< [out] Status code.
);

#endif // _ARCHIP_OPENCL_PROGRAM_FUN_H_

