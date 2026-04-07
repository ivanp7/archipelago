/**
 * @file
 * @brief Operations on OpenCL programs.
 */

#include "archi/opencl/api/program.fun.h"
#include "archi/opencl/api/device.fun.h"
#include "archi/opencl/api/string.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/util/string.fun.h"
#include "archi_log/print.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy()
#include <assert.h>


archi_opencl_program_src_t
archi_opencl_program_src_from_kvlist(
        const archi_kvlist_t *list,
        bool copy_buffers,
        ARCHI_ERROR_PARAM_DECL)
{
    archi_opencl_program_src_t sources = {0};

    // Count files and perform checks
    for (const archi_kvlist_t *node = list; node != NULL; node = node->next)
    {
        if (node->key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source name is NULL");
            return (archi_opencl_program_src_t){0};
        }
        else if (!archi_pointer_attr_compatible(node->value.attr,
                    ARCHI_POINTER_ATTR__PDATA(1, char)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source contents is not an array of bytes");
            return (archi_opencl_program_src_t){0};
        }

        archi_error_t error;
        if (!archi_pointer_valid(node->value, &error))
        {
            ARCHI_ERROR_SET(error.code, "source contents pointer is invalid: %s", error.message);
            return (archi_opencl_program_src_t){0};
        }

        sources.num_files++;
    }

    if (sources.num_files == 0)
    {
        ARCHI_ERROR_RESET();
        return sources;
    }

    // Allocate the storage
    sources.pathname = malloc(sizeof(*sources.pathname) * sources.num_files);
    if (sources.pathname == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of pathnames (length = %zu)",
                sources.num_files);
        goto failure;
    }

    for (size_t i = 0; i < sources.num_files; i++)
        sources.pathname[i] = NULL;

    sources.content = malloc(sizeof(*sources.content) * sources.num_files);
    if (sources.content == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of source contents (length = %zu)",
                sources.num_files);
        goto failure;
    }

    for (size_t i = 0; i < sources.num_files; i++)
        sources.content[i] = NULL;

    sources.size = malloc(sizeof(*sources.size) * sources.num_files);
    if (sources.size == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of source sizes (length = %zu)",
                sources.num_files);
        goto failure;
    }

    for (size_t i = 0; i < sources.num_files; i++)
        sources.size[i] = 0;

    // Fill the storage
    size_t index = 0;
    for (const archi_kvlist_t *node = list; node != NULL; node = node->next, index++)
    {
        size_t length;
        archi_pointer_attr_unpk__pdata(node->value.attr, &length, NULL, NULL, NULL);

        if (copy_buffers)
        {
            // Copy pathname
            sources.pathname[index] = archi_string_copy(node->key);
            if (sources.pathname[index] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate copy of pathname");
                goto failure;
            }

            // Copy source contents
            sources.content[index] = malloc(length);
            if (sources.content[index] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate copy of source contents");
                goto failure;
            }

            memcpy(sources.content[index], node->value.ptr, length);
        }
        else
        {
            sources.pathname[index] = (char*)node->key;
            sources.content[index] = node->value.ptr;
        }

        sources.size[index] = length;
    }

    ARCHI_ERROR_RESET();
    return sources;

failure:
    archi_opencl_program_src_free(sources, copy_buffers);

    return (archi_opencl_program_src_t){0};
}

archi_opencl_program_src_t
archi_opencl_program_src_copy(
        archi_opencl_program_src_t sources,
        bool copy_buffers,
        ARCHI_ERROR_PARAM_DECL)
{
    archi_opencl_program_src_t copy = {.num_files = sources.num_files};

    for (size_t i = 0; i < sources.num_files; i++)
    {
        if (sources.pathname[i] == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source name is NULL");
            return (archi_opencl_program_src_t){0};
        }
        else if (sources.content[i] == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source content is NULL");
            return (archi_opencl_program_src_t){0};
        }
        else if (sources.size[i] == 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source size is zero");
            return (archi_opencl_program_src_t){0};
        }
    }

    if (copy.num_files == 0)
    {
        ARCHI_ERROR_RESET();
        return copy;
    }

    // Allocate the storage
    copy.pathname = malloc(sizeof(*copy.pathname) * copy.num_files);
    if (copy.pathname == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of pathnames (length = %zu)",
                copy.num_files);
        goto failure;
    }

    for (size_t i = 0; i < copy.num_files; i++)
        copy.pathname[i] = NULL;

    copy.content = malloc(sizeof(*copy.content) * copy.num_files);
    if (copy.content == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of source contents (length = %zu)",
                copy.num_files);
        goto failure;
    }

    for (size_t i = 0; i < copy.num_files; i++)
        copy.content[i] = NULL;

    copy.size = malloc(sizeof(*copy.size) * copy.num_files);
    if (copy.size == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of source sizes (length = %zu)",
                copy.num_files);
        goto failure;
    }

    for (size_t i = 0; i < copy.num_files; i++)
        copy.size[i] = sources.size[i];

    // Fill the storage
    for (size_t i = 0; i < copy.num_files; i++)
    {
        if (copy_buffers)
        {
            // Copy pathname
            copy.pathname[i] = archi_string_copy(sources.pathname[i]);
            if (copy.pathname[i] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate copy of pathname");
                goto failure;
            }

            // Copy source contents
            copy.content[i] = malloc(copy.size[i]);
            if (copy.content[i] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate copy of source contents");
                goto failure;
            }

            memcpy(copy.content[i], sources.content[i], copy.size[i]);
        }
        else
        {
            copy.pathname[i] = sources.pathname[i];
            copy.content[i] = sources.content[i];
        }
    }

    ARCHI_ERROR_RESET();
    return copy;

failure:
    archi_opencl_program_src_free(copy, copy_buffers);

    return (archi_opencl_program_src_t){0};
}

void
archi_opencl_program_src_free(
        archi_opencl_program_src_t sources,
        bool free_buffers)
{
    if (free_buffers)
    {
        if (sources.pathname != NULL)
            for (size_t i = 0; i < sources.num_files; i++)
                free(sources.pathname[i]);

        if (sources.content != NULL)
            for (size_t i = 0; i < sources.num_files; i++)
                free(sources.content[i]);
    }

    free(sources.pathname);
    free(sources.content);
    free(sources.size);
}

static
void
archi_opencl_program_build_log(
        cl_program program,
        cl_uint num_devices,
        const cl_device_id device_id[])
{
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
                archi_log_debug(__func__, "[device #%u] couldn't obtain build status", i);
                continue;
            }

            const char *status_str = archi_opencl_string_build_status(status);

            if (status_str != NULL)
                archi_log_debug(__func__, "[device #%u] build status: %s", i, status_str);
            else
                archi_log_debug(__func__, "[device #%u] build status: code %i", i, status);
        }

        // Print program build log
        {
            size_t length;

            cl_int error = clGetProgramBuildInfo(program, device_id[i],
                    CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
            if (error != CL_SUCCESS)
            {
                archi_log_debug(__func__, "[device #%u] couldn't obtain length of build log", i);
                continue;
            }

            if (length > build_log_length)
            {
                char *new_build_log = realloc(build_log, length);
                if (new_build_log == NULL)
                {
                    archi_log_debug(__func__, "couldn't realloc(%zu bytes) for build log", length);
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
                archi_log_debug(__func__, "[device #%u] couldn't obtain build log", i);
                continue;
            }

            archi_log_debug(__func__, "[device #%u] build log: \n%s", i, build_log);
        }
    }

    free(build_log);
}

cl_program
archi_opencl_program_build(
        cl_context context,
        cl_uint num_devices,
        const cl_device_id device_id[],

        archi_opencl_program_src_t headers,
        archi_opencl_program_src_t sources,

        cl_uint num_libraries,
        const cl_program libraries[],

        const char *cflags,
        const char *lflags,

        bool logging,
        ARCHI_ERROR_PARAM_DECL)
{
    if (context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL context is NULL");
        return NULL;
    }
    else if (num_devices == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of devices to build program for is zero");
        return NULL;
    }
    else if (device_id == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of device IDs is NULL");
        return NULL;
    }
    else if ((num_libraries != 0) && (libraries == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of program libraries is NULL");
        return NULL;
    }
    else if (sources.num_files + num_libraries == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "total number of source files and libraries is zero");
        return NULL;
    }

    if (headers.num_files != 0)
    {
        if (headers.pathname == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of header pathnames is NULL");
            return NULL;
        }
        else if (headers.content == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of header contents is NULL");
            return NULL;
        }
        else if (headers.size == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of header sizes is NULL");
            return NULL;
        }

        for (size_t i = 0; i < headers.num_files; i++)
        {
            if (headers.pathname[i] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "header pathname is NULL");
                return NULL;
            }
            else if (headers.content[i] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "header content is NULL");
                return NULL;
            }
        }
    }

    if (sources.num_files != 0)
    {
        if (sources.pathname == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of source pathnames is NULL");
            return NULL;
        }
        else if (sources.content == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of source contents is NULL");
            return NULL;
        }
        else if (sources.size == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of source sizes is NULL");
            return NULL;
        }

        for (size_t i = 0; i < sources.num_files; i++)
        {
            if (sources.pathname[i] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source pathname is NULL");
                return NULL;
            }
            else if (sources.content[i] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source content is NULL");
                return NULL;
            }
        }
    }

    cl_program program = NULL;

    // Allocate the arrays
    cl_program *program_headers = NULL;
    cl_program *program_sources = NULL;

    if (headers.num_files > 0)
    {
        program_headers = malloc(sizeof(*program_headers) * headers.num_files);
        if (program_headers == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of program headers (length = %zu)",
                    headers.num_files);
            goto finish;
        }

        for (size_t i = 0; i < headers.num_files; i++)
            program_headers[i] = NULL;
    }

    {
        program_sources = malloc(sizeof(*program_sources) * (sources.num_files + num_libraries));
        if (program_sources == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of program sources (length = %zu)",
                    sources.num_files + num_libraries);
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
                1, (const char**)&headers.content[i], &headers.size[i], &ret);

        if (logging)
            archi_log_debug(__func__, "clCreateProgramWithSource('%s') -> %s",
                    headers.pathname[i], archi_opencl_string_error(ret));

        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create program with source: error %i", ret);
            goto finish;
        }
    }

    // Create source programs
    for (size_t i = 0; i < sources.num_files; i++)
    {
        cl_int ret;
        program_sources[i] = clCreateProgramWithSource(context,
                1, (const char**)&sources.content[i], &sources.size[i], &ret);

        if (logging)
            archi_log_debug(__func__, "clCreateProgramWithSource('%s') -> %s",
                    sources.pathname[i], archi_opencl_string_error(ret));

        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create program with source: error %i", ret);
            goto finish;
        }
    }

    // Compile source programs
    for (size_t i = 0; i < sources.num_files; i++)
    {
        cl_int ret = clCompileProgram(program_sources[i], num_devices, device_id, cflags,
                headers.num_files, program_headers, (const char**)headers.pathname, NULL, NULL);

        if (logging)
        {
            archi_log_debug(__func__, "clCompileProgram('%s') -> %s",
                    sources.pathname[i], archi_opencl_string_error(ret));

            archi_opencl_program_build_log(program_sources[i], num_devices, device_id);
        }

        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't compile program: error %i", ret);
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
            archi_log_debug(__func__, "clLinkProgram() -> %s", archi_opencl_string_error(ret));

            archi_opencl_program_build_log(program, num_devices, device_id);
        }

        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't link program: error %i", ret);
            goto finish;
        }
    }

    ARCHI_ERROR_RESET();

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
}

/*****************************************************************************/

archi_opencl_program_bin_t
archi_opencl_program_bin_extract(
        cl_program program,
        archi_opencl_platdev_t **out_platdev,
        ARCHI_ERROR_PARAM_DECL)
{
    if (program == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "program is NULL");
        return (archi_opencl_program_bin_t){0};
    }

    archi_opencl_program_bin_t binaries = {0};
    archi_opencl_platdev_t *platdev = NULL;

    // Get number of binaries (number of devices associated with the program)
    {
        cl_uint num_devices;

        cl_int ret = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES,
                sizeof(num_devices), &num_devices, NULL);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract number of devices the program was built for");
            goto failure;
        }
        else if (num_devices == 0) // probably never true, but check it anyway
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of devices the program was built for is zero");
            goto failure;
        }

        binaries.num_binaries = num_devices;
    }

    if (out_platdev != NULL)
    {
        // Allocate platform/device IDs storage object
        platdev = archi_opencl_platdev_alloc(binaries.num_binaries);
        if (platdev == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate platform/device IDs storage object (num_devices = %zu)",
                    binaries.num_binaries);
            goto failure;
        }

        // Get program devices IDs
        cl_int ret = clGetProgramInfo(program, CL_PROGRAM_DEVICES,
                sizeof(cl_device_id) * binaries.num_binaries, platdev->device_id, NULL);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract devices the program was built for");
            goto failure;
        }

        // Get program platform ID
        {
            cl_int ret = clGetDeviceInfo(platdev->device_id[0], CL_DEVICE_PLATFORM,
                    sizeof(cl_platform_id), &platdev->platform_id, NULL);
            if (ret != CL_SUCCESS)
            {
                ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract platform ID associated with a device");
                goto failure;
            }
        }
    }

    // Get sizes of binaries
    {
        size_t binary_sizes_size;

        cl_int ret = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, 0, NULL, &binary_sizes_size);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract size of the array of sizes of program binaries");
            goto failure;
        }
        else if (binary_sizes_size == 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "size of the array of sizes of program binaries is zero");
            goto failure;
        }

        binaries.size = malloc(binary_sizes_size);
        if (binaries.size == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of sizes of program binaries (length = %zu)",
                    binary_sizes_size / sizeof(size_t));
            goto failure;
        }

        ret = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, binary_sizes_size, binaries.size, NULL);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract sizes of program binaries");
            goto failure;
        }
    }

    // Get contents of binaries
    {
        size_t contents_size = sizeof(*binaries.content) * binaries.num_binaries;

        binaries.content = malloc(contents_size);
        if (binaries.content == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of pointers to program binaries (length = %zu)",
                    binaries.num_binaries);
            goto failure;
        }

        for (cl_uint i = 0; i < binaries.num_binaries; i++)
            binaries.content[i] = NULL;

        for (cl_uint i = 0; i < binaries.num_binaries; i++)
        {
            if (binaries.size[i] == 0)
                continue;

            binaries.content[i] = malloc(binaries.size[i]);
            if (binaries.content[i] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate buffer for program binary (size = %zu)",
                        binaries.size[i]);
                goto failure;
            }
        }

        cl_int ret = clGetProgramInfo(program, CL_PROGRAM_BINARIES, contents_size, binaries.content, NULL);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract program binaries");
            goto failure;
        }
    }

    if (out_platdev != NULL)
        *out_platdev = platdev;

    ARCHI_ERROR_RESET();
    return binaries;

failure:
    archi_opencl_program_bin_free(binaries, true);
    free(platdev);

    return (archi_opencl_program_bin_t){0};
}

archi_opencl_program_bin_t
archi_opencl_program_bin_copy(
        archi_opencl_program_bin_t binaries,
        bool copy_buffers,
        ARCHI_ERROR_PARAM_DECL)
{
    archi_opencl_program_bin_t copy = {.num_binaries = binaries.num_binaries};

    for (size_t i = 0; i < binaries.num_binaries; i++)
    {
        if (binaries.content[i] == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "binary content is NULL");
            return (archi_opencl_program_bin_t){0};
        }
        else if (binaries.size[i] == 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "binary size is zero");
            return (archi_opencl_program_bin_t){0};
        }
    }

    if (copy.num_binaries == 0)
    {
        ARCHI_ERROR_RESET();
        return copy;
    }

    // Allocate the storage
    copy.content = malloc(sizeof(*copy.content) * copy.num_binaries);
    if (copy.content == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of binary contents (length = %zu)",
                copy.num_binaries);
        goto failure;
    }

    for (size_t i = 0; i < copy.num_binaries; i++)
        copy.content[i] = NULL;

    copy.size = malloc(sizeof(*copy.size) * copy.num_binaries);
    if (copy.size == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of binary sizes (length = %zu)",
                copy.num_binaries);
        goto failure;
    }

    for (size_t i = 0; i < copy.num_binaries; i++)
        copy.size[i] = binaries.size[i];

    // Fill the storage
    for (size_t i = 0; i < copy.num_binaries; i++)
    {
        if (copy_buffers)
        {
            // Copy binary contents
            copy.content[i] = malloc(copy.size[i]);
            if (copy.content[i] == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate copy of binary contents (%zu bytes)",
                        copy.size[i]);
                goto failure;
            }

            memcpy(copy.content[i], binaries.content[i], copy.size[i]);
        }
        else
            copy.content[i] = binaries.content[i];
    }

    ARCHI_ERROR_RESET();
    return copy;

failure:
    archi_opencl_program_bin_free(copy, copy_buffers);

    return (archi_opencl_program_bin_t){0};
}

void
archi_opencl_program_bin_free(
        archi_opencl_program_bin_t binaries,
        bool free_buffers)
{
    if (free_buffers)
    {
        if (binaries.content != NULL)
            for (cl_uint i = 0; i < binaries.num_binaries; i++)
                free(binaries.content[i]);
    }

    free(binaries.content);
    free(binaries.size);
}

cl_program
archi_opencl_program_create(
        cl_context context,

        const archi_opencl_platdev_t *platdev,
        archi_opencl_program_bin_t binaries,
        bool build,

        bool logging,
        ARCHI_ERROR_PARAM_DECL)
{
    if (context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL context is NULL");
        return NULL;
    }
    else if (platdev == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to platform/device IDs storage object is NULL");
        return NULL;
    }
    else if (platdev->num_devices == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of devices is zero");
        return NULL;
    }
    else if (binaries.num_binaries != platdev->num_devices)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "number of binaries is not equal to number of devices");
        return NULL;
    }
    else if (binaries.content == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of pointers to binaries is NULL");
        return NULL;
    }
    else if (binaries.size == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "array of binaries sizes is NULL");
        return NULL;
    }

    for (size_t i = 0; i < binaries.num_binaries; i++)
        if (binaries.content[i] == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to binary is NULL");
            return NULL;
        }

    cl_program program = NULL;

    // Create the program from binary
    {
        cl_int ret;
        cl_int *binary_status = NULL;

        if (logging)
        {
            binary_status = malloc(sizeof(*binary_status) * platdev->num_devices);
            if (binary_status == NULL)
            {
                ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of program binary statuses (length = %u)",
                        platdev->num_devices);
                return NULL;
            }
        }

        program = clCreateProgramWithBinary(context,
                platdev->num_devices, platdev->device_id,
                binaries.size, (const unsigned char**)binaries.content, binary_status, &ret);

        if (logging)
        {
            archi_log_debug(__func__, "clCreateProgramWithBinary() -> %s", archi_opencl_string_error(ret));

            for (cl_uint i = 0; i < platdev->num_devices; i++)
                archi_log_debug(__func__, "[device #%u] status: %s", i, archi_opencl_string_error(binary_status[i]));

            free(binary_status);
        }

        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create program from binaries: error %i", ret);
            return NULL;
        }
    }

    if (build)
    {
        // Build the program
        cl_int ret;

        ret = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

        if (logging)
        {
            archi_log_debug(__func__, "clBuildProgram() -> %s", archi_opencl_string_error(ret));

            archi_opencl_program_build_log(program, platdev->num_devices, platdev->device_id);
        }

        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't build program: error %i", ret);
            return NULL;
        }
    }

    ARCHI_ERROR_RESET();
    return program;
}

