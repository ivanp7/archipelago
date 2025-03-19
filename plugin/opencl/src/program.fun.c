/**
 * @file
 * @brief Building OpenCL programs from sources.
 */

#include "opencl/program.fun.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy()

const char*
plugin_opencl_program_concat_flags(
        const char *flags[])
{
    if (flags == NULL)
        return NULL;

    size_t total_length = 0;
    for (size_t i = 0; flags[i] != NULL; i++)
        total_length += strlen(flags[i]) + 1 /* space */;

    char *result = malloc(total_length);
    if (result == NULL)
        return NULL;

    size_t processed_length = 0;
    for (size_t i = 0; flags[i] != NULL; i++)
    {
        size_t length = strlen(flags[i]);
        memcpy(result + processed_length, flags[i], length);
        processed_length += length + 1;
        result[processed_length - 1] = ' ';
    }

    result[total_length - 1] = '\0';
    return result;
}

static
const char*
plugin_opencl_error_string(
        cl_int err)
{
    switch (err)
    {
#define CASE_ERROR_STR(x) case x: return #x;

        // run-time and JIT compiler errors
        CASE_ERROR_STR(CL_SUCCESS)
        CASE_ERROR_STR(CL_DEVICE_NOT_FOUND)
        CASE_ERROR_STR(CL_DEVICE_NOT_AVAILABLE)
        CASE_ERROR_STR(CL_COMPILER_NOT_AVAILABLE)
        CASE_ERROR_STR(CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CASE_ERROR_STR(CL_OUT_OF_RESOURCES)
        CASE_ERROR_STR(CL_OUT_OF_HOST_MEMORY)
        CASE_ERROR_STR(CL_PROFILING_INFO_NOT_AVAILABLE)
        CASE_ERROR_STR(CL_MEM_COPY_OVERLAP)
        CASE_ERROR_STR(CL_IMAGE_FORMAT_MISMATCH)
        CASE_ERROR_STR(CL_IMAGE_FORMAT_NOT_SUPPORTED)
        CASE_ERROR_STR(CL_BUILD_PROGRAM_FAILURE)
        CASE_ERROR_STR(CL_MAP_FAILURE)
        CASE_ERROR_STR(CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CASE_ERROR_STR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CASE_ERROR_STR(CL_COMPILE_PROGRAM_FAILURE)
        CASE_ERROR_STR(CL_LINKER_NOT_AVAILABLE)
        CASE_ERROR_STR(CL_LINK_PROGRAM_FAILURE)
        CASE_ERROR_STR(CL_DEVICE_PARTITION_FAILED)
        CASE_ERROR_STR(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)

        // compile-time errors
        CASE_ERROR_STR(CL_INVALID_VALUE)
        CASE_ERROR_STR(CL_INVALID_DEVICE_TYPE)
        CASE_ERROR_STR(CL_INVALID_PLATFORM)
        CASE_ERROR_STR(CL_INVALID_DEVICE)
        CASE_ERROR_STR(CL_INVALID_CONTEXT)
        CASE_ERROR_STR(CL_INVALID_QUEUE_PROPERTIES)
        CASE_ERROR_STR(CL_INVALID_COMMAND_QUEUE)
        CASE_ERROR_STR(CL_INVALID_HOST_PTR)
        CASE_ERROR_STR(CL_INVALID_MEM_OBJECT)
        CASE_ERROR_STR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
        CASE_ERROR_STR(CL_INVALID_IMAGE_SIZE)
        CASE_ERROR_STR(CL_INVALID_SAMPLER)
        CASE_ERROR_STR(CL_INVALID_BINARY)
        CASE_ERROR_STR(CL_INVALID_BUILD_OPTIONS)
        CASE_ERROR_STR(CL_INVALID_PROGRAM)
        CASE_ERROR_STR(CL_INVALID_PROGRAM_EXECUTABLE)
        CASE_ERROR_STR(CL_INVALID_KERNEL_NAME)
        CASE_ERROR_STR(CL_INVALID_KERNEL_DEFINITION)
        CASE_ERROR_STR(CL_INVALID_KERNEL)
        CASE_ERROR_STR(CL_INVALID_ARG_INDEX)
        CASE_ERROR_STR(CL_INVALID_ARG_VALUE)
        CASE_ERROR_STR(CL_INVALID_ARG_SIZE)
        CASE_ERROR_STR(CL_INVALID_KERNEL_ARGS)
        CASE_ERROR_STR(CL_INVALID_WORK_DIMENSION)
        CASE_ERROR_STR(CL_INVALID_WORK_GROUP_SIZE)
        CASE_ERROR_STR(CL_INVALID_WORK_ITEM_SIZE)
        CASE_ERROR_STR(CL_INVALID_GLOBAL_OFFSET)
        CASE_ERROR_STR(CL_INVALID_EVENT_WAIT_LIST)
        CASE_ERROR_STR(CL_INVALID_EVENT)
        CASE_ERROR_STR(CL_INVALID_OPERATION)
        CASE_ERROR_STR(CL_INVALID_GL_OBJECT)
        CASE_ERROR_STR(CL_INVALID_BUFFER_SIZE)
        CASE_ERROR_STR(CL_INVALID_MIP_LEVEL)
        CASE_ERROR_STR(CL_INVALID_GLOBAL_WORK_SIZE)
        CASE_ERROR_STR(CL_INVALID_PROPERTY)
        CASE_ERROR_STR(CL_INVALID_IMAGE_DESCRIPTOR)
        CASE_ERROR_STR(CL_INVALID_COMPILER_OPTIONS)
        CASE_ERROR_STR(CL_INVALID_LINKER_OPTIONS)
        CASE_ERROR_STR(CL_INVALID_DEVICE_PARTITION_COUNT)

#undef CASE_ERROR_STR

        default: return NULL;
    }
}

#define M "plugin_opencl_program_build"

static
void
plugin_opencl_program_build_log(
        cl_program program,
        cl_uint num_devices,
        const cl_device_id *device_ids)
{
    char *build_log = NULL;
    size_t build_log_length = 0;

    for (cl_uint i = 0; i < num_devices; i++)
    {
        // Print program build status
        {
            cl_build_status status;

            cl_int error = clGetProgramBuildInfo(program, device_ids[i],
                    CL_PROGRAM_BUILD_STATUS, sizeof(status), &status, NULL);
            if (error != CL_SUCCESS)
            {
                archi_log_debug(M, "[device #%u] couldn't obtain build status", i);
                continue;
            }

            const char *status_str = NULL;
            switch (error)
            {
#define CASE_STATUS_STR(x) case x: status_str = #x; break

                CASE_STATUS_STR(CL_BUILD_NONE);
                CASE_STATUS_STR(CL_BUILD_ERROR);
                CASE_STATUS_STR(CL_BUILD_SUCCESS);
                CASE_STATUS_STR(CL_BUILD_IN_PROGRESS);

#undef CASE_STATUS_STR
            }

            if (status_str != NULL)
                archi_log_debug(M, "[device #%u] build status: %s", i, status_str);
            else
                archi_log_debug(M, "[device #%u] build status: code %i", i, status);
        }

        // Print program build log
        {
            size_t length;

            cl_int error = clGetProgramBuildInfo(program, device_ids[i],
                    CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
            if (error != CL_SUCCESS)
            {
                archi_log_debug(M, "[device #%u] couldn't obtain length of build log", i);
                continue;
            }

            if (length > build_log_length)
            {
                char *new_build_log = realloc(build_log, length);
                if (new_build_log == NULL)
                {
                    archi_log_debug(M, "couldn't realloc(%llu bytes) for build log", length);
                    continue;
                }

                build_log = new_build_log;
                build_log_length = length;
            }

            if (length == 0)
                continue;

            error = clGetProgramBuildInfo(program, device_ids[i],
                    CL_PROGRAM_BUILD_LOG, length, build_log, NULL);
            if (error != CL_SUCCESS)
            {
                archi_log_debug(M, "[device #%u] couldn't obtain build log", i);
                continue;
            }

            archi_log_debug(M, "[device #%u] build status: \n%s", i, build_log);
        }
    }

    free(build_log);
}

cl_program
plugin_opencl_program_build(
        cl_context context,
        cl_uint num_devices,
        const cl_device_id *device_ids,

        cl_uint num_header_sets,
        const plugin_opencl_program_files_t *header_sets[],

        cl_uint num_source_sets,
        const plugin_opencl_program_files_t *source_sets[],

        cl_uint num_libraries,
        const cl_program libraries[],

        const char *cflags,
        const char *lflags,

        bool logging,
        archi_status_t *code)
{
    if ((context == NULL) || ((num_devices > 0) && (device_ids == NULL)) ||
            ((num_header_sets > 0) && (header_sets == NULL)) ||
            ((num_source_sets > 0) && (source_sets == NULL)) ||
            ((num_libraries > 0) && (libraries == NULL)) ||
            ((num_source_sets == 0) && (num_libraries == 0)))
    {
        if (code != NULL)
            *code = ARCHI_ERROR_MISUSE;

        return NULL;
    }

    const char **header_include_names = NULL;

    cl_program *program_headers = NULL;
    cl_program *program_sources = NULL;
    cl_program program = NULL;

    // Count headers and sources
    cl_uint num_headers = 0;
    for (cl_uint i = 0; i < num_header_sets; i++)
        num_headers += header_sets[i]->num_files;

    cl_uint num_sources = 0;
    for (cl_uint i = 0; i < num_source_sets; i++)
        num_sources += source_sets[i]->num_files;

    // Allocate the memory
    if (num_headers > 0)
    {
        header_include_names = malloc(sizeof(*header_include_names) * num_headers);
        if (header_include_names == NULL)
        {
            if (code != NULL)
                *code = ARCHI_ERROR_ALLOC;

            goto finish;
        }

        program_headers = malloc(sizeof(*program_headers) * num_headers);
        if (program_headers == NULL)
        {
            if (code != NULL)
                *code = ARCHI_ERROR_ALLOC;

            goto finish;
        }

        for (cl_uint idx = 0; idx < num_headers; idx++)
            program_headers[idx] = NULL;
    }

    {
        program_sources = malloc(sizeof(*program_sources) * (num_sources + num_libraries));
        if (program_sources == NULL)
        {
            if (code != NULL)
                *code = ARCHI_ERROR_ALLOC;

            goto finish;
        }

        for (cl_uint idx = 0; idx < num_sources; idx++)
            program_sources[idx] = NULL;

        for (cl_uint idx = 0; idx < num_libraries; idx++)
            program_sources[num_sources + idx] = libraries[idx];
    }

    // Create header programs
    if (num_headers > 0)
    {
        cl_uint idx = 0;
        for (cl_uint i = 0; i < num_header_sets; i++)
        {
            if ((header_sets[i]->contents == NULL) || (header_sets[i]->sizes == NULL) ||
                    (header_sets[i]->pathnames == NULL))
            {
                if (code != NULL)
                    *code = ARCHI_ERROR_MISUSE;

                goto finish;
            }

            for (cl_uint j = 0; j < header_sets[i]->num_files; j++)
            {
                header_include_names[idx] = header_sets[i]->pathnames[j];

                cl_int err;
                program_headers[idx++] = clCreateProgramWithSource(context,
                        1, &header_sets[i]->contents[j], &header_sets[i]->sizes[j], &err);

                if (logging)
                    archi_log_debug(M, "clCreateProgramWithSource('%s') -> %s",
                            header_sets[i]->pathnames[j], plugin_opencl_error_string(err));

                if (err != CL_SUCCESS)
                {
                    if (code != NULL)
                        *code = ARCHI_ERROR_OPERATION;

                    goto finish;
                }
            }
        }
    }

    // Create source programs
    {
        cl_uint idx = 0;
        for (cl_uint i = 0; i < num_source_sets; i++)
        {
            if ((source_sets[i]->contents == NULL) || (source_sets[i]->sizes == NULL) ||
                    (source_sets[i]->pathnames == NULL))
            {
                if (code != NULL)
                    *code = ARCHI_ERROR_MISUSE;

                goto finish;
            }

            for (cl_uint j = 0; j < source_sets[i]->num_files; j++)
            {
                cl_int err;
                program_sources[idx++] = clCreateProgramWithSource(context,
                        1, &source_sets[i]->contents[j], &source_sets[i]->sizes[j], &err);

                if (logging)
                    archi_log_debug(M, "clCreateProgramWithSource('%s') -> %s",
                            source_sets[i]->pathnames[j], plugin_opencl_error_string(err));

                if (err != CL_SUCCESS)
                {
                    if (code != NULL)
                        *code = ARCHI_ERROR_OPERATION;

                    goto finish;
                }
            }
        }
    }

    // Compile source programs
    {
        cl_uint idx = 0;
        for (cl_uint i = 0; i < num_source_sets; i++)
        {
            for (cl_uint j = 0; j < source_sets[i]->num_files; j++, idx++)
            {
                cl_int err = clCompileProgram(program_sources[idx], num_devices, device_ids, cflags,
                        num_headers, program_headers, header_include_names, NULL, NULL);

                if (logging)
                {
                    archi_log_debug(M, "clCompileProgram('%s') -> %s",
                            source_sets[i]->pathnames[j], plugin_opencl_error_string(err));

                    plugin_opencl_program_build_log(program_sources[idx], num_devices, device_ids);
                }

                if (err != CL_SUCCESS)
                {
                    if (code != NULL)
                        *code = ARCHI_ERROR_OPERATION;

                    goto finish;
                }
            }
        }
    }

    // Link the program
    {
        cl_int err;

        program = clLinkProgram(context, num_devices, device_ids, lflags,
                num_sources + num_libraries, program_sources, NULL, NULL, &err);

        if (logging)
        {
            archi_log_debug(M, "clLinkProgram() -> %s", plugin_opencl_error_string(err));

            plugin_opencl_program_build_log(program, num_devices, device_ids);
        }

        if (err != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_ERROR_OPERATION;

            goto finish;
        }
    }

    if (code != NULL)
        *code = 0;

finish:
    if (program_sources != NULL)
    {
        for (cl_uint i = 0; i < num_sources; i++)
            if (program_sources[i] != NULL)
                clReleaseProgram(program_sources[i]);

        free(program_sources);
    }

    if (program_headers != NULL)
    {
        for (cl_uint i = 0; i < num_headers; i++)
            if (program_headers[i] != NULL)
                clReleaseProgram(program_headers[i]);

        free(program_headers);
    }

    free(header_include_names);

    return program;
}

#undef M
#define M "plugin_opencl_program_create"

cl_program
plugin_opencl_program_create(
        cl_context context,

        plugin_opencl_program_binaries_t binaries,

        bool logging,
        archi_status_t *code)
{
    if ((context == NULL) || (binaries.num_devices == 0) || (binaries.device_ids == NULL) ||
            (binaries.contents == NULL) || (binaries.sizes == NULL))
    {
        if (code != NULL)
            *code = ARCHI_ERROR_MISUSE;

        return NULL;
    }

    cl_int err;
    cl_int *binary_status = NULL;

    if (logging)
    {
        binary_status = malloc(sizeof(*binary_status) * binaries.num_devices);
        if (binary_status == NULL)
        {
            if (code != NULL)
                *code = ARCHI_ERROR_ALLOC;

            return NULL;
        }
    }

    cl_program program = clCreateProgramWithBinary(context, binaries.num_devices, binaries.device_ids,
            binaries.sizes, binaries.contents, binary_status, &err);

    if (logging)
    {
        archi_log_debug(M, "clCreateProgramWithBinary() -> %s", plugin_opencl_error_string(err));

        for (cl_uint i = 0; i < binaries.num_devices; i++)
            archi_log_debug(M, "[device #%u] status: %s", i, plugin_opencl_error_string(binary_status[i]));

        free(binary_status);
    }

    if (err != CL_SUCCESS)
    {
        if (code != NULL)
            *code = ARCHI_ERROR_OPERATION;

        return NULL;
    }

    if (code != NULL)
        *code = 0;

    return program;
}

archi_status_t
plugin_opencl_program_binaries_extract(
        cl_program program,
        plugin_opencl_program_binaries_t *binaries)
{
    if ((program == NULL) || (binaries == NULL))
        return ARCHI_ERROR_MISUSE;

    *binaries = (plugin_opencl_program_binaries_t){0};

    archi_status_t code;
    cl_int err;

    // Get platform ID
    {
        cl_context context = NULL;
        err = clGetProgramInfo(program, CL_PROGRAM_CONTEXT, sizeof(context), &context, NULL);
        if (err != CL_SUCCESS)
        {
            code = ARCHI_ERROR_OPERATION;
            goto failure;
        }

        size_t properties_size = 0;
        err = clGetContextInfo(context, CL_CONTEXT_PROPERTIES, 0, NULL, &properties_size);
        if (err != CL_SUCCESS)
        {
            code = ARCHI_ERROR_OPERATION;
            goto failure;
        }
        else if (properties_size == 0) // platform ID is not specified
        {
            code = ARCHI_ERROR_UNAVAIL;
            goto failure;
        }

        cl_context_properties *properties = malloc(properties_size);
        if (properties == NULL)
        {
            code = ARCHI_ERROR_ALLOC;
            goto failure;
        }

        err = clGetContextInfo(context, CL_CONTEXT_PROPERTIES, properties_size, properties, NULL);
        if (err != CL_SUCCESS)
        {
            free(properties);
            code = ARCHI_ERROR_OPERATION;
            goto failure;
        }

        cl_context_properties *property = properties;
        while ((char*)property[0] != NULL)
        {
            if (property[0] == CL_CONTEXT_PLATFORM)
            {
                binaries->platform_id = (cl_platform_id)property[1];
                break;
            }

            property += 2; // name and value
        }

        free(properties);

        if (*property == NULL) // platform ID is not specified
        {
            code = ARCHI_ERROR_UNAVAIL;
            goto failure;
        }
    }

    // Get device IDs
    {
        err = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(binaries->num_devices),
                &binaries->num_devices, NULL);
        if (err != CL_SUCCESS)
        {
            code = ARCHI_ERROR_OPERATION;
            goto failure;
        }

        size_t device_ids_size = 0;
        err = clGetProgramInfo(program, CL_PROGRAM_DEVICES, 0, NULL, &device_ids_size);
        if (err != CL_SUCCESS)
        {
            code = ARCHI_ERROR_OPERATION;
            goto failure;
        }
        else if (device_ids_size == 0)
        {
            code = ARCHI_ERROR_UNAVAIL;
            goto failure;
        }

        binaries->device_ids = malloc(device_ids_size);
        if (binaries->device_ids == NULL)
        {
            code = ARCHI_ERROR_ALLOC;
            goto failure;
        }

        err = clGetProgramInfo(program, CL_PROGRAM_DEVICES, device_ids_size, binaries->device_ids, NULL);
        if (err != CL_SUCCESS)
        {
            code = ARCHI_ERROR_OPERATION;
            goto failure;
        }
    }

    // Get sizes of binaries
    {
        size_t binary_sizes_size = 0;
        err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, 0, NULL, &binary_sizes_size);
        if (err != CL_SUCCESS)
        {
            code = ARCHI_ERROR_OPERATION;
            goto failure;
        }
        else if (binary_sizes_size == 0)
        {
            code = ARCHI_ERROR_UNAVAIL;
            goto failure;
        }

        binaries->sizes = malloc(binary_sizes_size);
        if (binaries->sizes == NULL)
        {
            code = ARCHI_ERROR_ALLOC;
            goto failure;
        }

        err = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, binary_sizes_size, binaries->sizes, NULL);
        if (err != CL_SUCCESS)
        {
            code = ARCHI_ERROR_OPERATION;
            goto failure;
        }
    }

    // Get binaries
    {
        size_t contents_size = sizeof(*binaries->contents) * binaries->num_devices;

        binaries->contents = malloc(contents_size);
        if (binaries->contents == NULL)
        {
            code = ARCHI_ERROR_ALLOC;
            goto failure;
        }

        for (cl_uint i = 0; i < binaries->num_devices; i++)
            binaries->contents[i] = NULL;

        for (cl_uint i = 0; i < binaries->num_devices; i++)
        {
            if (binaries->sizes[i] == 0)
                continue;

            binaries->contents[i] = malloc(binaries->sizes[i]);
            if (binaries->contents[i] == NULL)
            {
                code = ARCHI_ERROR_ALLOC;
                goto failure;
            }
        }

        err = clGetProgramInfo(program, CL_PROGRAM_BINARIES, contents_size, binaries->contents, NULL);
        if (err != CL_SUCCESS)
        {
            code = ARCHI_ERROR_OPERATION;
            goto failure;
        }
    }

    return 0;

failure:
    plugin_opencl_program_binaries_free(binaries);
    return code;
}

void
plugin_opencl_program_binaries_free(
        plugin_opencl_program_binaries_t *binaries)
{
    if (binaries == NULL)
        return;

    if (binaries->contents != NULL)
    {
        for (cl_uint i = 0; i < binaries->num_devices; i++)
            free(binaries->contents[i]);

        free(binaries->contents);
    }

    free(binaries->sizes);
    free(binaries->device_ids);

    *binaries = (plugin_opencl_program_binaries_t){0};
}

