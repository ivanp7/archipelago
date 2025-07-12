/**
 * @file
 * @brief Application context interfaces for OpenCL programs.
 */

#include "archip/opencl/ctx/program.var.h"
#include "archip/opencl/program.fun.h"
#include "archi/log/print.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), memcpy()
#include <stdalign.h> // for alignof()

#include <CL/cl.h>

struct archip_context_opencl_program_data {
    archi_pointer_t program;

    archi_pointer_t context;
    archip_opencl_program_binaries_t binaries;
};

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_program_init_src)
{
    archi_pointer_t opencl_context = {0};
    archi_pointer_t device_id = {0};
    archi_pointer_t hashmap_headers = {0};
    archi_pointer_t hashmap_sources = {0};
    archi_pointer_t array_libraries = {0};
    const char *cflags = NULL;
    const char *lflags = NULL;

    bool param_opencl_context_set = false;
    bool param_device_ids_set = false;
    bool param_headers_set = false;
    bool param_sources_set = false;
    bool param_array_libraries_set = false;
    bool param_cflags_set = false;
    bool param_lflags_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("context", params->name) == 0)
        {
            if (param_opencl_context_set)
                continue;
            param_opencl_context_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            opencl_context = params->value;
        }
        else if (strcmp("device_id", params->name) == 0)
        {
            if (param_device_ids_set)
                continue;
            param_device_ids_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            device_id = params->value;
        }
        else if (strcmp("headers", params->name) == 0)
        {
            if (param_headers_set)
                continue;
            param_headers_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            hashmap_headers = params->value;
        }
        else if (strcmp("sources", params->name) == 0)
        {
            if (param_sources_set)
                continue;
            param_sources_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            hashmap_sources = params->value;
        }
        else if (strcmp("libraries", params->name) == 0)
        {
            if (param_array_libraries_set)
                continue;
            param_array_libraries_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            array_libraries = params->value;
        }
        else if (strcmp("cflags", params->name) == 0)
        {
            if (param_cflags_set)
                continue;
            param_cflags_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            cflags = params->value.ptr;
        }
        else if (strcmp("lflags", params->name) == 0)
        {
            if (param_lflags_set)
                continue;
            param_lflags_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            lflags = params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archip_context_opencl_program_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archip_opencl_program_sources_t headers;
    {
        archi_status_t code;
        headers = archip_opencl_program_sources_from_hashmap(hashmap_headers.ptr, &code);
        if (code != 0)
        {
            free(context_data);
            return code;
        }
    }

    archip_opencl_program_sources_t sources;
    {
        archi_status_t code;
        sources = archip_opencl_program_sources_from_hashmap(hashmap_sources.ptr, &code);
        if (code != 0)
        {
            archip_opencl_program_sources_free(headers);
            free(context_data);
            return code;
        }
    }

    archi_status_t code;
    cl_program program = archip_opencl_program_build(opencl_context.ptr,
            device_id.element.num_of, device_id.ptr, headers, sources,
            array_libraries.element.num_of, array_libraries.ptr, cflags, lflags,
            true, &code);

    archip_opencl_program_sources_free(headers);
    archip_opencl_program_sources_free(sources);

    if (code != 0)
    {
        free(context_data);
        return code;
    }

    archip_opencl_program_binaries_t binaries =
        archip_opencl_program_binaries_extract(program, &code);

    if (code != 0)
    {
        clReleaseProgram(program);
        free(context_data);
        return code;
    }

    *context_data = (struct archip_context_opencl_program_data){
        .program = {
            .ptr = program,
            .element = {
                .num_of = 1,
            },
        },
        .context = opencl_context,
        .binaries = binaries,
    };

    archi_reference_count_increment(opencl_context.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_program_init_bin)
{
    archi_pointer_t opencl_context = {0};
    archi_pointer_t device_id = {0};
    archi_pointer_t array_binaries = {0};

    bool param_opencl_context_set = false;
    bool param_device_ids_set = false;
    bool param_binaries_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("context", params->name) == 0)
        {
            if (param_opencl_context_set)
                continue;
            param_opencl_context_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            opencl_context = params->value;
        }
        else if (strcmp("device_id", params->name) == 0)
        {
            if (param_device_ids_set)
                continue;
            param_device_ids_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            device_id = params->value;
        }
        else if (strcmp("binaries", params->name) == 0)
        {
            if (param_binaries_set)
                continue;
            param_binaries_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            array_binaries = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (array_binaries.element.num_of != device_id.element.num_of)
        return ARCHI_STATUS_EMISUSE;

    struct archip_context_opencl_program_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archip_opencl_program_binaries_t binaries;
    {
        archi_status_t code;
        binaries = archip_opencl_program_binaries_from_array(
                array_binaries.ptr, array_binaries.element.num_of, &code);
        if (code != 0)
        {
            free(context_data);
            return code;
        }

        // Get program platform ID
        if (device_id.ptr != NULL)
        {
            cl_int ret = clGetDeviceInfo(*(cl_device_id*)device_id.ptr, CL_DEVICE_PLATFORM,
                    sizeof(cl_platform_id), &binaries.ids->platform_id, NULL);
            if (ret != CL_SUCCESS)
            {
                archip_opencl_program_binaries_free(binaries);
                free(context_data);
                return ARCHI_STATUS_ERESOURCE;
            }
        }

        // Copy program device IDs
        memcpy(binaries.ids->device_id, device_id.ptr,
                sizeof(cl_device_id) * device_id.element.num_of);
    }

    archi_status_t code;
    cl_program program = archip_opencl_program_create(
            opencl_context.ptr, binaries, true, &code);

    if (code != 0)
    {
        archip_opencl_program_binaries_free(binaries);
        free(context_data);
        return code;
    }

    *context_data = (struct archip_context_opencl_program_data){
        .program = {
            .ptr = program,
            .element = {
                .num_of = 1,
            },
        },
        .context = opencl_context,
        .binaries = binaries,
    };

    archi_reference_count_increment(opencl_context.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_program_final)
{
    struct archip_context_opencl_program_data *context_data =
        (struct archip_context_opencl_program_data*)context;

    clReleaseProgram(context_data->program.ptr);

    archi_reference_count_decrement(context_data->context.ref_count);
    archip_opencl_program_binaries_free(context_data->binaries);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_program_get)
{
    struct archip_context_opencl_program_data *context_data =
        (struct archip_context_opencl_program_data*)context;

    if (strcmp("context", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->context;
    }
    else if (strcmp("platform_id", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = context_data->binaries.ids->platform_id,
            .ref_count = context_data->program.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->binaries.ids->platform_id),
                .alignment = alignof(cl_platform_id),
            },
        };
    }
    else if (strcmp("device_id", slot.name) == 0)
    {
        if (slot.num_indices > 1)
            return ARCHI_STATUS_EMISUSE;

        if (slot.num_indices == 0)
        {
            *value = (archi_pointer_t){
                .ptr = (context_data->binaries.ids->num_devices > 0) ?
                    context_data->binaries.ids->device_id : NULL,
                .ref_count = context_data->program.ref_count,
                .element = {
                    .num_of = context_data->binaries.ids->num_devices,
                    .size = sizeof(context_data->binaries.ids->device_id[0]),
                    .alignment = alignof(cl_device_id),
                },
            };
        }
        else
        {
            if (slot.index[0] >= context_data->binaries.ids->num_devices)
                return ARCHI_STATUS_EMISUSE;

            *value = (archi_pointer_t){
                .ptr = context_data->binaries.ids->device_id[slot.index[0]],
                .ref_count = context_data->program.ref_count,
                .element = {
                    .num_of = 1,
                    .size = sizeof(context_data->binaries.ids->device_id[0]),
                    .alignment = alignof(cl_device_id),
                },
            };
        }
    }
    else if (strcmp("binary_size", slot.name) == 0)
    {
        if (slot.num_indices > 1)
            return ARCHI_STATUS_EMISUSE;

        if (slot.num_indices == 0)
        {
            *value = (archi_pointer_t){
                .ptr = context_data->binaries.sizes,
                .ref_count = context_data->program.ref_count,
                .element = {
                    .num_of = context_data->binaries.ids->num_devices,
                    .size = sizeof(context_data->binaries.sizes[0]),
                    .alignment = alignof(size_t),
                },
            };
        }
        else
        {
            if (slot.index[0] >= context_data->binaries.ids->num_devices)
                return ARCHI_STATUS_EMISUSE;

            *value = (archi_pointer_t){
                .ptr = &context_data->binaries.sizes[slot.index[0]],
                .ref_count = context_data->program.ref_count,
                .element = {
                    .num_of = 1,
                    .size = sizeof(context_data->binaries.sizes[0]),
                    .alignment = alignof(size_t),
                },
            };
        }
    }
    else if (strcmp("binary", slot.name) == 0)
    {
        if (slot.num_indices > 1)
            return ARCHI_STATUS_EMISUSE;

        if (slot.num_indices == 0)
        {
            *value = (archi_pointer_t){
                .ptr = context_data->binaries.contents,
                .ref_count = context_data->program.ref_count,
                .element = {
                    .num_of = context_data->binaries.ids->num_devices,
                    .size = sizeof(context_data->binaries.contents[0]),
                    .alignment = alignof(unsigned char*),
                },
            };
        }
        else
        {
            if (slot.index[0] >= context_data->binaries.ids->num_devices)
                return ARCHI_STATUS_EMISUSE;

            *value = (archi_pointer_t){
                .ptr = context_data->binaries.contents[slot.index[0]],
                .ref_count = context_data->program.ref_count,
                .element = {
                    .num_of = context_data->binaries.sizes[slot.index[0]],
                    .size = sizeof(context_data->binaries.contents[0][0]),
                    .alignment = alignof(unsigned char),
                },
            };
        }
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archip_context_opencl_program_src_interface = {
    .init_fn = archip_context_opencl_program_init_src,
    .final_fn = archip_context_opencl_program_final,
    .get_fn = archip_context_opencl_program_get,
};

const archi_context_interface_t archip_context_opencl_program_bin_interface = {
    .init_fn = archip_context_opencl_program_init_bin,
    .final_fn = archip_context_opencl_program_final,
    .get_fn = archip_context_opencl_program_get,
};

/*****************************************************************************/

struct archip_context_opencl_kernel_data {
    archi_pointer_t kernel;

    archi_pointer_t program;
    char *kernel_name;
    cl_uint num_arguments;
};

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_kernel_init_new)
{
#define M "archip_context_opencl_kernel_init_new"

    archi_pointer_t program = {0};
    char *kernel_name = NULL;

    bool param_program_set = false;
    bool param_kernel_name_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("program", params->name) == 0)
        {
            if (param_program_set)
                continue;
            param_program_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            program = params->value;
        }
        else if (strcmp("name", params->name) == 0)
        {
            if (param_kernel_name_set)
                continue;
            param_kernel_name_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            kernel_name = params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archip_context_opencl_kernel_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    {
        size_t name_len = strlen(kernel_name) + 1;
        char *kernel_name_copy = malloc(name_len);
        if (kernel_name_copy == NULL)
        {
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        memcpy(kernel_name_copy, kernel_name, name_len);

        kernel_name = kernel_name_copy;
    }

    cl_int ret;
    cl_kernel kernel = clCreateKernel(program.ptr, kernel_name, &ret);
    if (ret != CL_SUCCESS)
    {
        archi_log_error(M, "clCreateKernel('%s') failed with error %i",
                kernel_name, ret);
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    cl_uint num_arguments;
    {
        ret = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS,
                sizeof(num_arguments), &num_arguments, NULL);
        if (ret != CL_SUCCESS)
        {
            archi_log_error(M, "clGetKernelInfo(CL_KERNEL_NUM_ARGS) failed with error %i", ret);
            clReleaseKernel(kernel);
            free(context_data);
            return ARCHI_STATUS_ERESOURCE;
        }
    }

    *context_data = (struct archip_context_opencl_kernel_data){
        .kernel = {
            .ptr = kernel,
            .element = {
                .num_of = 1,
            },
        },
        .program = program,
        .kernel_name = kernel_name,
        .num_arguments = num_arguments,
    };

    archi_reference_count_increment(program.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;

#undef M
}

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_kernel_init_copy)
{
#define M "archip_context_opencl_kernel_init_copy"

    archi_pointer_t source_kernel = {0};

    bool param_source_kernel_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("kernel", params->name) == 0)
        {
            if (param_source_kernel_set)
                continue;
            param_source_kernel_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            source_kernel = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archip_context_opencl_kernel_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    cl_int ret;
    cl_kernel kernel = clCloneKernel(source_kernel.ptr, &ret);
    if (ret != CL_SUCCESS)
    {
        archi_log_error(M, "clCloneKernel() failed with error %i", ret);
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    cl_program program;
    ret = clGetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(program), &program, NULL);
    if (ret != CL_SUCCESS)
    {
        archi_log_error(M, "clGetKernelInfo(CL_KERNEL_PROGRAM) failed with error %i", ret);
        clReleaseKernel(kernel);
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    char *kernel_name;
    {
        size_t name_len;
        ret = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, 0, NULL, &name_len);
        if (ret != CL_SUCCESS)
        {
            archi_log_error(M, "clGetKernelInfo(CL_KERNEL_FUNCTION_NAME) failed with error %i", ret);
            clReleaseKernel(kernel);
            free(context_data);
            return ARCHI_STATUS_ERESOURCE;
        }

        kernel_name = malloc(name_len);
        if (kernel_name == NULL)
        {
            clReleaseKernel(kernel);
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        ret = clGetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, name_len, kernel_name, NULL);
        if (ret != CL_SUCCESS)
        {
            archi_log_error(M, "clGetKernelInfo(CL_KERNEL_FUNCTION_NAME) failed with error %i", ret);
            free(kernel_name);
            clReleaseKernel(kernel);
            free(context_data);
            return ARCHI_STATUS_ERESOURCE;
        }
    }

    cl_uint num_arguments;
    {
        ret = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS,
                sizeof(num_arguments), &num_arguments, NULL);
        if (ret != CL_SUCCESS)
        {
            archi_log_error(M, "clGetKernelInfo(CL_KERNEL_NUM_ARGS) failed with error %i", ret);
            clReleaseKernel(kernel);
            free(context_data);
            return ARCHI_STATUS_ERESOURCE;
        }
    }

    *context_data = (struct archip_context_opencl_kernel_data){
        .kernel = {
            .ptr = kernel,
            .element = {
                .num_of = 1,
            },
        },
        .program = {
            .ptr = program,
            .ref_count = source_kernel.ref_count,
            .element = {
                .num_of = 1,
            },
        },
        .kernel_name = kernel_name,
        .num_arguments = num_arguments,
    };

    archi_reference_count_increment(source_kernel.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;

#undef M
}

ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_kernel_final)
{
    struct archip_context_opencl_kernel_data *context_data =
        (struct archip_context_opencl_kernel_data*)context;

    clReleaseKernel(context_data->kernel.ptr);
    archi_reference_count_decrement(context_data->program.ref_count);
    free(context_data->kernel_name);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_kernel_get)
{
    struct archip_context_opencl_kernel_data *context_data =
        (struct archip_context_opencl_kernel_data*)context;

    if (strcmp("program", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_pointer_t program = context_data->program;
        program.ref_count = context_data->kernel.ref_count;

        *value = program;
    }
    else if (strcmp("kernel_name", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = context_data->kernel_name,
            .ref_count = context_data->kernel.ref_count,
            .element = {
                .num_of = strlen(context_data->kernel_name) + 1,
                .size = 1,
                .alignment = 1,
            },
        };
    }
    else if (strcmp("num_arguments", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &context_data->num_arguments,
            .ref_count = context_data->kernel.ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(context_data->num_arguments),
                .alignment = alignof(cl_uint),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archip_context_opencl_kernel_set)
{
    struct archip_context_opencl_kernel_data *context_data =
        (struct archip_context_opencl_kernel_data*)context;

    if (strcmp("arg.value", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;
        else if (value.ptr == NULL)
            return ARCHI_STATUS_EMISUSE;
        else if ((value.element.num_of == 0) || (value.element.size == 0))
            return ARCHI_STATUS_EMISUSE;

        ptrdiff_t arg_index = slot.index[0];
        if ((arg_index < 0) || ((cl_uint)arg_index >= context_data->num_arguments))
            return ARCHI_STATUS_EMISUSE;

        size_t arg_size = value.element.num_of * value.element.size;
        if (arg_size / value.element.size != value.element.num_of)
            return ARCHI_STATUS_EVALUE;

        cl_int ret = clSetKernelArg(context_data->kernel.ptr, arg_index, arg_size, value.ptr);
        if (ret != CL_SUCCESS)
        {
            archi_log_error("archip_context_opencl_kernel_set", "clSetKernelArg(%ti) failed with error %i", arg_index, ret);
            return ARCHI_STATUS_ERESOURCE;
        }
    }
    else if (strcmp("arg.svm_ptr", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;
        else if (value.ptr == NULL)
            return ARCHI_STATUS_EMISUSE;

        ptrdiff_t arg_index = slot.index[0];
        if ((arg_index < 0) || ((cl_uint)arg_index >= context_data->num_arguments))
            return ARCHI_STATUS_EMISUSE;

        cl_int ret = clSetKernelArgSVMPointer(context_data->kernel.ptr, arg_index, value.ptr);
        if (ret != CL_SUCCESS)
        {
            archi_log_error("archip_context_opencl_kernel_set", "clSetKernelArgSVMPointer(%ti) failed with error %i", arg_index, ret);
            return ARCHI_STATUS_ERESOURCE;
        }
    }
    else if (strcmp("exec_info.svm_ptrs", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;
        else if (value.element.size != sizeof(void*))
            return ARCHI_STATUS_EMISUSE;

        cl_int ret = clSetKernelExecInfo(context_data->kernel.ptr, CL_KERNEL_EXEC_INFO_SVM_PTRS,
                sizeof(void*) * value.element.num_of, value.ptr);
        if (ret != CL_SUCCESS)
        {
            archi_log_error("archip_context_opencl_kernel_set", "clSetKernelExecInfo(CL_KERNEL_EXEC_INFO_SVM_PTRS) failed with error %i", ret);
            return ARCHI_STATUS_ERESOURCE;
        }
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archip_context_opencl_kernel_new_interface = {
    .init_fn = archip_context_opencl_kernel_init_new,
    .final_fn = archip_context_opencl_kernel_final,
    .get_fn = archip_context_opencl_kernel_get,
    .set_fn = archip_context_opencl_kernel_set,
};

const archi_context_interface_t archip_context_opencl_kernel_copy_interface = {
    .init_fn = archip_context_opencl_kernel_init_copy,
    .final_fn = archip_context_opencl_kernel_final,
    .get_fn = archip_context_opencl_kernel_get,
    .set_fn = archip_context_opencl_kernel_set,
};

