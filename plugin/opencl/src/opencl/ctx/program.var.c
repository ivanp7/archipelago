/**
 * @file
 * @brief Application context interfaces for OpenCL programs.
 */

#include "archip/opencl/ctx/program.var.h"
#include "archip/opencl/program.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), memcpy()
#include <stdalign.h> // for alignof()

#include <CL/cl.h>

struct archip_context_opencl_program_data {
    archi_pointer_t program;
    archi_pointer_t context;
    archi_pointer_t device_id;
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

    if (device_id.ptr != NULL)
    {
        size_t size = sizeof(cl_device_id) * device_id.element.num_of;

        void *device_list = malloc(size);
        if (device_list == NULL)
        {
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        memcpy(device_list, device_id.ptr, size);

        device_id = (archi_pointer_t){
            .ptr = device_list,
            .element = {
                .num_of = device_id.element.num_of,
                .size = sizeof(cl_device_id),
                .alignment = alignof(cl_device_id),
            },
        };
    }

    archip_opencl_program_sources_t headers;
    {
        archi_status_t code;
        headers = archip_opencl_program_sources_from_hashmap(hashmap_headers.ptr, &code);
        if (code != 0)
        {
            free(device_id.ptr);
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
            free(device_id.ptr);
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
        free(device_id.ptr);
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
        .device_id = device_id,
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

    if (device_id.ptr != NULL)
    {
        size_t size = sizeof(cl_device_id) * device_id.element.num_of;

        void *device_list = malloc(size);
        if (device_list == NULL)
        {
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        memcpy(device_list, device_id.ptr, size);

        device_id = (archi_pointer_t){
            .ptr = device_list,
            .element = {
                .num_of = device_id.element.num_of,
                .size = sizeof(cl_device_id),
                .alignment = alignof(cl_device_id),
            },
        };
    }

    archip_opencl_program_binaries_t binaries;
    {
        archi_status_t code;
        binaries = archip_opencl_program_binaries_from_array(array_binaries.ptr,
                array_binaries.element.num_of, &code);
        if (code != 0)
        {
            free(device_id.ptr);
            free(context_data);
            return code;
        }

        memcpy(binaries.ids->device_id, device_id.ptr,
                sizeof(cl_device_id) * device_id.element.num_of);
    }

    archi_status_t code;
    cl_program program = archip_opencl_program_create(
            opencl_context.ptr, binaries, true, &code);

    archip_opencl_program_binaries_free(binaries);

    if (code != 0)
    {
        free(device_id.ptr);
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
        .device_id = device_id,
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
    free(context_data->device_id.ptr);
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
    else if (strcmp("device_id", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->device_id;
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
    archi_pointer_t source_kernel;
    cl_uint num_arguments;
};

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_kernel_init_new)
{
    archi_pointer_t program = {0};
    const char *kernel_name = NULL;

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

    cl_int ret;
    cl_kernel kernel = clCreateKernel(program.ptr, kernel_name, &ret);
    if (ret != CL_SUCCESS)
    {
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    *context_data = (struct archip_context_opencl_kernel_data){
        .kernel = {
            .ptr = kernel,
            .element = {
                .num_of = 1,
            },
        },
        .program = program,
    };

    ret = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS,
            sizeof(context_data->num_arguments), &context_data->num_arguments, NULL);
    if (ret != CL_SUCCESS)
    {
        clReleaseKernel(kernel);
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    archi_reference_count_increment(program.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_kernel_init_copy)
{
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
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    cl_program program;
    ret = clGetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(program), &program, NULL);
    if (ret != CL_SUCCESS)
    {
        clReleaseKernel(kernel);
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
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
            .element = {
                .num_of = 1,
            },
        },
        .source_kernel = source_kernel,
    };

    ret = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS,
            sizeof(context_data->num_arguments), &context_data->num_arguments, NULL);
    if (ret != CL_SUCCESS)
    {
        clReleaseKernel(kernel);
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    archi_reference_count_increment(source_kernel.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_kernel_final)
{
    struct archip_context_opencl_kernel_data *context_data =
        (struct archip_context_opencl_kernel_data*)context;

    clReleaseKernel(context_data->kernel.ptr);
    archi_reference_count_decrement(context_data->program.ref_count);
    archi_reference_count_decrement(context_data->source_kernel.ref_count);
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
        if (program.ref_count == NULL)
            program.ref_count = context_data->source_kernel.ref_count;

        *value = program;
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
            return ARCHI_STATUS_ERESOURCE;
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
            return ARCHI_STATUS_ERESOURCE;
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
            return ARCHI_STATUS_ERESOURCE;
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

