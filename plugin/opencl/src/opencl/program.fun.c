/**
 * @file
 * @brief Operations on OpenCL programs.
 */

#include "archip/opencl/program.fun.h"
#include "archi/ds/hashmap/api.fun.h"
#include "archi/util/size.def.h"
#include "archi/log/print.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy(), strlen()

const char*
archip_opencl_program_concat_flags(
        const char *flags[])
{
    if (flags == NULL)
        return NULL;

    size_t total_length = 0;
    for (size_t i = 0; flags[i] != NULL; i++)
        total_length += strlen(flags[i]) + 1 /* space or terminator */;

    char *result = malloc(total_length);
    if (result == NULL)
        return NULL;

    size_t position = 0;
    for (size_t i = 0; flags[i] != NULL; i++)
    {
        size_t length = strlen(flags[i]);

        memcpy(result + position, flags[i], length);
        result[position + length] = ' ';

        position += length + 1;
    }
    result[total_length - 1] = '\0';

    return result;
}

static
char*
archip_opencl_copy_key(
        const char *key)
{
    if (key == NULL)
        return NULL;

    size_t size = strlen(key) + 1;

    char *key_copy = malloc(size);
    if (key_copy == NULL)
        return NULL;

    memcpy(key_copy, key, size);

    return key_copy;
}

static
ARCHI_HASHMAP_TRAV_KV_FUNC(archip_opencl_program_sources_from_hashmap_traverse_func)
{
    archip_opencl_program_sources_t *sources = data;

    size_t size = value.element.num_of;
    if (value.element.size != 0)
    {
        size *= value.element.size;
        if (size / value.element.size != value.element.num_of)
            return (archi_hashmap_trav_action_t){.interrupt = true};
    }

    if ((value.ptr == NULL) || (size == 0))
        return (archi_hashmap_trav_action_t){.interrupt = true};

    char *key_copy = archip_opencl_copy_key(key);
    if (key_copy == NULL)
        return (archi_hashmap_trav_action_t){.interrupt = true};

    char *contents = malloc(size);
    if (contents == NULL)
    {
        free(key_copy);
        return (archi_hashmap_trav_action_t){.interrupt = true};
    }

    memcpy(contents, value.ptr, size);

    sources->pathnames[index] = key_copy;
    sources->sizes[index] = size;
    sources->contents[index] = contents;

    return (archi_hashmap_trav_action_t){0};
}

archip_opencl_program_sources_t
archip_opencl_program_sources_from_hashmap(
        void *hashmap,
        archi_status_t *code)
{
    if (hashmap == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return (archip_opencl_program_sources_t){0};
    }

    archip_opencl_program_sources_t sources = {0};

    sources.num_files = archi_hashmap_size(hashmap);
    if (sources.num_files == 0)
    {
        if (code != NULL)
            *code = 0;

        return sources;
    }

    sources.pathnames = malloc(sizeof(*sources.pathnames) * sources.num_files);
    if (sources.pathnames == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        goto failure;
    }

    for (size_t i = 0; i < sources.num_files; i++)
        sources.pathnames[i] = NULL;

    sources.sizes = malloc(sizeof(*sources.sizes) * sources.num_files);
    if (sources.sizes == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        goto failure;
    }

    sources.contents = malloc(sizeof(*sources.contents) * sources.num_files);
    if (sources.contents == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        goto failure;
    }

    for (size_t i = 0; i < sources.num_files; i++)
        sources.contents[i] = NULL;

    archi_status_t trav_code = archi_hashmap_traverse(hashmap, true,
            archip_opencl_program_sources_from_hashmap_traverse_func, &sources);
    if (trav_code < 0)
    {
        if (code != NULL)
            *code = trav_code;

        goto failure;
    }
    else if (trav_code == 1)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        goto failure;
    }

    if (code != NULL)
        *code = 0;

    return sources;

failure:
    archip_opencl_program_sources_free(sources);

    return (archip_opencl_program_sources_t){0};
}

void
archip_opencl_program_sources_free(
        archip_opencl_program_sources_t sources)
{
    if (sources.contents != NULL)
    {
        for (size_t i = 0; i < sources.num_files; i++)
            free(sources.contents[i]);

        free(sources.contents);
    }

    if (sources.pathnames != NULL)
    {
        for (size_t i = 0; i < sources.num_files; i++)
            free(sources.pathnames[i]);

        free(sources.pathnames);
    }

    free(sources.sizes);
}

archip_opencl_program_binaries_t
archip_opencl_program_binaries_from_array(
        archi_pointer_t *array,
        size_t num_elements,
        archi_status_t *code)
{
    if (array == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return (archip_opencl_program_binaries_t){0};
    }

    archip_opencl_program_binaries_t binaries = {0};

    binaries.ids = malloc(ARCHI_SIZEOF_FLEXIBLE(
                archip_opencl_platform_device_ids_t, device_id, num_elements));
    if (binaries.ids == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return (archip_opencl_program_binaries_t){0};
    }

    binaries.ids->platform_id = NULL;
    binaries.ids->num_devices = num_elements;
    for (size_t i = 0; i < num_elements; i++)
        binaries.ids->device_id[i] = NULL;

    if (num_elements == 0)
    {
        if (code != NULL)
            *code = 0;

        return binaries;
    }

    binaries.sizes = malloc(sizeof(*binaries.sizes) * num_elements);
    if (binaries.sizes == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        goto failure;
    }

    for (size_t i = 0; i < num_elements; i++)
    {
        size_t size = array[i].element.num_of * array[i].element.size;

        if (array[i].element.size == 0)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_EVALUE;

            goto failure;
        }
        else if (size / array[i].element.size != array[i].element.num_of)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            goto failure;
        }

        binaries.sizes[i] = size;
    }

    binaries.contents = malloc(sizeof(*binaries.contents) * num_elements);
    if (binaries.contents == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        goto failure;
    }

    for (size_t i = 0; i < num_elements; i++)
        binaries.contents[i] = NULL;

    for (size_t i = 0; i < num_elements; i++)
    {
        if (binaries.sizes[i] == 0)
            continue;

        binaries.contents[i] = malloc(binaries.sizes[i]);
        if (binaries.contents[i] == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            goto failure;
        }

        memcpy(binaries.contents[i], array[i].ptr, binaries.sizes[i]);
    }

    if (code != NULL)
        *code = 0;

    return binaries;

failure:
    archip_opencl_program_binaries_free(binaries);

    return (archip_opencl_program_binaries_t){0};
}

void
archip_opencl_program_binaries_free(
        archip_opencl_program_binaries_t binaries)
{
    if (binaries.contents != NULL)
    {
        for (cl_uint i = 0; i < binaries.ids->num_devices; i++)
            free(binaries.contents[i]);

        free(binaries.contents);
    }

    free(binaries.sizes);
    free(binaries.ids);
}

static
const char*
archip_opencl_error_string(
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

static
void
archip_opencl_program_build_log(
        cl_program program,
        cl_uint num_devices,
        const cl_device_id device_id[])
{
#define M "archip_opencl_program_build"

    char *build_log = NULL;
    size_t build_log_length = 0;

    for (cl_uint i = 0; i < num_devices; i++)
    {
        // Print program build status
        {
            cl_build_status status;

            cl_int error = clGetProgramBuildInfo(program, device_id[i],
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

            cl_int error = clGetProgramBuildInfo(program, device_id[i],
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
                    archi_log_debug(M, "couldn't realloc(%zu bytes) for build log", length);
                    continue;
                }

                build_log = new_build_log;
                build_log_length = length;
            }

            if (length == 0)
                continue;

            error = clGetProgramBuildInfo(program, device_id[i],
                    CL_PROGRAM_BUILD_LOG, length, build_log, NULL);
            if (error != CL_SUCCESS)
            {
                archi_log_debug(M, "[device #%u] couldn't obtain build log", i);
                continue;
            }

            archi_log_debug(M, "[device #%u] build log: \n%s", i, build_log);
        }
    }

    free(build_log);

#undef M
}

cl_program
archip_opencl_program_build(
        cl_context context,
        cl_uint num_devices,
        const cl_device_id device_id[],

        archip_opencl_program_sources_t headers,
        archip_opencl_program_sources_t sources,

        cl_uint num_libraries,
        const cl_program libraries[],

        const char *cflags,
        const char *lflags,

        bool logging,
        archi_status_t *code)
{
#define M "archip_opencl_program_build"

    if ((context == NULL) || ((num_devices > 0) && (device_id == NULL)) ||
            ((headers.num_files > 0) && ((headers.pathnames == NULL) ||
                (headers.sizes == NULL) || (headers.contents == NULL))) ||
            ((sources.num_files > 0) && ((sources.pathnames == NULL) ||
                (sources.sizes == NULL) || (sources.contents == NULL))) ||
            ((num_libraries > 0) && (libraries == NULL)) ||
            (sources.num_files + num_libraries == 0))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    cl_program *program_headers = NULL;
    cl_program *program_sources = NULL;
    cl_program program = NULL;

    // Allocate the arrays
    if (headers.num_files > 0)
    {
        program_headers = malloc(sizeof(*program_headers) * headers.num_files);
        if (program_headers == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            goto finish;
        }

        for (size_t i = 0; i < headers.num_files; i++)
            program_headers[i] = NULL;
    }

    {
        program_sources = malloc(sizeof(*program_sources) * (sources.num_files + num_libraries));
        if (program_sources == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            goto finish;
        }

        for (size_t i = 0; i < sources.num_files; i++)
            program_sources[i] = NULL;

        for (size_t i = 0; i < num_libraries; i++)
            program_sources[sources.num_files + i] = libraries[i];
    }

    // Create header programs
    for (size_t i = 0; i < headers.num_files; i++)
    {
        cl_int ret;
        program_headers[i] = clCreateProgramWithSource(context,
                1, (const char**)&headers.contents[i], &headers.sizes[i], &ret);

        if (logging)
            archi_log_debug(M, "clCreateProgramWithSource('%s') -> %s",
                    headers.pathnames[i], archip_opencl_error_string(ret));

        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto finish;
        }
    }

    // Create source programs
    for (size_t i = 0; i < sources.num_files; i++)
    {
        cl_int ret;
        program_sources[i] = clCreateProgramWithSource(context,
                1, (const char**)&sources.contents[i], &sources.sizes[i], &ret);

        if (logging)
            archi_log_debug(M, "clCreateProgramWithSource('%s') -> %s",
                    sources.pathnames[i], archip_opencl_error_string(ret));

        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto finish;
        }
    }

    // Compile source programs
    for (size_t i = 0; i < sources.num_files; i++)
    {
        cl_int ret = clCompileProgram(program_sources[i], num_devices, device_id, cflags,
                headers.num_files, program_headers, (const char**)headers.pathnames, NULL, NULL);

        if (logging)
        {
            archi_log_debug(M, "clCompileProgram('%s') -> %s",
                    sources.pathnames[i], archip_opencl_error_string(ret));

            archip_opencl_program_build_log(program_sources[i], num_devices, device_id);
        }

        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto finish;
        }
    }

    // Link the program
    {
        cl_int ret;

        program = clLinkProgram(context, num_devices, device_id, lflags,
                sources.num_files + num_libraries, program_sources, NULL, NULL, &ret);

        if (logging)
        {
            archi_log_debug(M, "clLinkProgram() -> %s", archip_opencl_error_string(ret));

            archip_opencl_program_build_log(program, num_devices, device_id);
        }

        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto finish;
        }
    }

    if (code != NULL)
        *code = 0;

finish:
    if (program_sources != NULL)
    {
        for (cl_uint i = 0; i < sources.num_files; i++)
            if (program_sources[i] != NULL)
                clReleaseProgram(program_sources[i]);

        free(program_sources);
    }

    if (program_headers != NULL)
    {
        for (cl_uint i = 0; i < headers.num_files; i++)
            if (program_headers[i] != NULL)
                clReleaseProgram(program_headers[i]);

        free(program_headers);
    }

    return program;

#undef M
}

cl_program
archip_opencl_program_create(
        cl_context context,

        archip_opencl_program_binaries_t binaries,

        bool logging,
        archi_status_t *code)
{
#define M "archip_opencl_program_create"

    if ((context == NULL) || (binaries.ids == NULL) ||
            (binaries.ids->num_devices == 0) ||
            (binaries.sizes == NULL) || (binaries.contents == NULL))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    cl_int ret;
    cl_int *binary_status = NULL;

    if (logging)
    {
        binary_status = malloc(sizeof(*binary_status) * binaries.ids->num_devices);
        if (binary_status == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            return NULL;
        }
    }

    cl_program program = clCreateProgramWithBinary(context,
            binaries.ids->num_devices, binaries.ids->device_id,
            binaries.sizes, (const unsigned char**)binaries.contents, binary_status, &ret);

    if (logging)
    {
        archi_log_debug(M, "clCreateProgramWithBinary() -> %s", archip_opencl_error_string(ret));

        for (cl_uint i = 0; i < binaries.ids->num_devices; i++)
            archi_log_debug(M, "[device #%u] status: %s", i, archip_opencl_error_string(binary_status[i]));

        free(binary_status);
    }

    if (ret != CL_SUCCESS)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ERESOURCE;

        return NULL;
    }

    if (code != NULL)
        *code = 0;

    return program;

#undef M
}

archip_opencl_program_binaries_t
archip_opencl_program_binaries_extract(
        cl_program program,
        archi_status_t *code)
{
    if (program == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return (archip_opencl_program_binaries_t){0};
    }

    archip_opencl_program_binaries_t binaries = {0};

    // Get program devices and platform ID
    {
        cl_uint num_devices;

        cl_int ret = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES,
                sizeof(num_devices), &num_devices, NULL);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }
        else if (num_devices == 0) // probably never true, but check it anyway
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }

        binaries.ids = malloc(ARCHI_SIZEOF_FLEXIBLE(
                    archip_opencl_platform_device_ids_t, device_id, num_devices));
        if (binaries.ids == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            goto failure;
        }

        binaries.ids->num_devices = num_devices;

        ret = clGetProgramInfo(program, CL_PROGRAM_DEVICES,
                sizeof(cl_device_id) * num_devices, binaries.ids->device_id, NULL);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }
    }

    // Get program platform ID
    {
        cl_int ret = clGetDeviceInfo(binaries.ids->device_id[0], CL_DEVICE_PLATFORM,
                sizeof(cl_platform_id), &binaries.ids->platform_id, NULL);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }
    }

    // Get sizes of binaries
    {
        size_t binary_sizes_size;

        cl_int ret = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, 0, NULL, &binary_sizes_size);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }
        else if (binary_sizes_size == 0)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }

        binaries.sizes = malloc(binary_sizes_size);
        if (binaries.sizes == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            goto failure;
        }

        ret = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, binary_sizes_size, binaries.sizes, NULL);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }
    }

    // Get contents of binaries
    {
        size_t contents_size = sizeof(*binaries.contents) * binaries.ids->num_devices;

        binaries.contents = malloc(contents_size);
        if (binaries.contents == NULL)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ENOMEMORY;

            goto failure;
        }

        for (cl_uint i = 0; i < binaries.ids->num_devices; i++)
            binaries.contents[i] = NULL;

        for (cl_uint i = 0; i < binaries.ids->num_devices; i++)
        {
            if (binaries.sizes[i] == 0)
                continue;

            binaries.contents[i] = malloc(binaries.sizes[i]);
            if (binaries.contents[i] == NULL)
            {
                if (code != NULL)
                    *code = ARCHI_STATUS_ENOMEMORY;

                goto failure;
            }
        }

        cl_int ret = clGetProgramInfo(program, CL_PROGRAM_BINARIES, contents_size, binaries.contents, NULL);
        if (ret != CL_SUCCESS)
        {
            if (code != NULL)
                *code = ARCHI_STATUS_ERESOURCE;

            goto failure;
        }
    }

    if (code != NULL)
        *code = 0;

    return binaries;

failure:
    archip_opencl_program_binaries_free(binaries);

    return (archip_opencl_program_binaries_t){0};
}

