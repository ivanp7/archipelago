/**
 * @file
 * @brief Application context interfaces for OpenCL kernels.
 */

#include "archi/opencl/ctx/kernel.var.h"
#include "archipelago/base/ref_count.fun.h"
#include "archipelago/util/alloc.fun.h"
#include "archipelago/log/print.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

#include <CL/cl.h>

struct archi_context_opencl_kernel_data {
    archi_pointer_t kernel;

    archi_pointer_t program;
    char *kernel_name;
    cl_uint num_arguments;
};

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_kernel_init_new)
{
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

    struct archi_context_opencl_kernel_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    if (kernel_name != NULL)
    {
        char *kernel_name_copy = archi_copy_string(kernel_name);
        if (kernel_name_copy == NULL)
        {
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        kernel_name = kernel_name_copy;
    }

    cl_int ret;
    cl_kernel kernel = clCreateKernel(program.ptr, kernel_name, &ret);
    if (ret != CL_SUCCESS)
    {
        archi_log_error(__func__, "clCreateKernel('%s') failed with error %i",
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
            archi_log_error(__func__, "clGetKernelInfo(CL_KERNEL_NUM_ARGS) failed with error %i", ret);
            clReleaseKernel(kernel);
            free(context_data);
            return ARCHI_STATUS_ERESOURCE;
        }
    }

    *context_data = (struct archi_context_opencl_kernel_data){
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
}

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_kernel_init_clone)
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

    struct archi_context_opencl_kernel_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    cl_int ret;
    cl_kernel kernel = clCloneKernel(source_kernel.ptr, &ret);
    if (ret != CL_SUCCESS)
    {
        archi_log_error(__func__, "clCloneKernel() failed with error %i", ret);
        free(context_data);
        return ARCHI_STATUS_ERESOURCE;
    }

    cl_program program;
    ret = clGetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(program), &program, NULL);
    if (ret != CL_SUCCESS)
    {
        archi_log_error(__func__, "clGetKernelInfo(CL_KERNEL_PROGRAM) failed with error %i", ret);
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
            archi_log_error(__func__, "clGetKernelInfo(CL_KERNEL_FUNCTION_NAME) failed with error %i", ret);
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
            archi_log_error(__func__, "clGetKernelInfo(CL_KERNEL_FUNCTION_NAME) failed with error %i", ret);
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
            archi_log_error(__func__, "clGetKernelInfo(CL_KERNEL_NUM_ARGS) failed with error %i", ret);
            clReleaseKernel(kernel);
            free(context_data);
            return ARCHI_STATUS_ERESOURCE;
        }
    }

    *context_data = (struct archi_context_opencl_kernel_data){
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
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_kernel_final)
{
    struct archi_context_opencl_kernel_data *context_data =
        (struct archi_context_opencl_kernel_data*)context;

    clReleaseKernel(context_data->kernel.ptr);
    archi_reference_count_decrement(context_data->program.ref_count);
    free(context_data->kernel_name);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_kernel_get)
{
    struct archi_context_opencl_kernel_data *context_data =
        (struct archi_context_opencl_kernel_data*)context;

    if (strcmp("program", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_pointer_t program = context_data->program;
        program.ref_count = context->ref_count;

        *value = program;
    }
    else if (strcmp("name", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = context_data->kernel_name,
            .ref_count = context->ref_count,
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
            .ref_count = context->ref_count,
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

ARCHI_CONTEXT_SET_FUNC(archi_context_opencl_kernel_set)
{
    struct archi_context_opencl_kernel_data *context_data =
        (struct archi_context_opencl_kernel_data*)context;

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
            archi_log_error("archi_context_opencl_kernel_set", "clSetKernelArg(%ti) failed with error %i", arg_index, ret);
            return ARCHI_STATUS_ERESOURCE;
        }
    }
    else if (strcmp("arg.mem_ptr", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EMISUSE;

        ptrdiff_t arg_index = slot.index[0];
        if ((arg_index < 0) || ((cl_uint)arg_index >= context_data->num_arguments))
            return ARCHI_STATUS_EMISUSE;

        cl_int ret = clSetKernelArg(context_data->kernel.ptr, arg_index, sizeof(cl_mem), value.ptr);
        if (ret != CL_SUCCESS)
        {
            archi_log_error("archi_context_opencl_kernel_set", "clSetKernelArg(%ti) failed with error %i", arg_index, ret);
            return ARCHI_STATUS_ERESOURCE;
        }
    }
    else if (strcmp("arg.local_mem_size", slot.name) == 0)
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

        size_t arg_size = *(size_t*)value.ptr;

        cl_int ret = clSetKernelArg(context_data->kernel.ptr, arg_index, arg_size, NULL);
        if (ret != CL_SUCCESS)
        {
            archi_log_error("archi_context_opencl_kernel_set", "clSetKernelArg(%ti) failed with error %i", arg_index, ret);
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
            archi_log_error("archi_context_opencl_kernel_set", "clSetKernelArgSVMPointer(%ti) failed with error %i", arg_index, ret);
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
            archi_log_error("archi_context_opencl_kernel_set", "clSetKernelExecInfo(CL_KERNEL_EXEC_INFO_SVM_PTRS) failed with error %i", ret);
            return ARCHI_STATUS_ERESOURCE;
        }
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_opencl_kernel_new_interface = {
    .init_fn = archi_context_opencl_kernel_init_new,
    .final_fn = archi_context_opencl_kernel_final,
    .get_fn = archi_context_opencl_kernel_get,
    .set_fn = archi_context_opencl_kernel_set,
};

const archi_context_interface_t archi_context_opencl_kernel_clone_interface = {
    .init_fn = archi_context_opencl_kernel_init_clone,
    .final_fn = archi_context_opencl_kernel_final,
    .get_fn = archi_context_opencl_kernel_get,
    .set_fn = archi_context_opencl_kernel_set,
};

