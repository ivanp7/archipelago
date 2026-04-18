/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Context interface for OpenCL kernels.
 */

#include "archi/opencl/ctx/kernel.var.h"
#include "archi/opencl/api/tag.def.h"
#include "archi/context/api/interface.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <CL/cl.h>

#include <stdlib.h> // for malloc(), free()


struct archi_context_data__opencl_kernel {
    archi_rcpointer_t kernel;
    char *kernel_name;
    cl_uint num_arguments;

    // References
    archi_rcpointer_t ref_program;
};

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__opencl_kernel_new)
{
    // Parse parameters
    archi_rcpointer_t program = {0};
    const char *kernel_name = NULL;
    {
        archi_plist_param_t parsed[] = {
            {.name = "program",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM)}},
                .assign = {archi_plist_assign__rcpointer, &program, sizeof(program), NULL}},
            {.name = "name",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(2, char)}},
                .assign = {archi_plist_assign__dptr, &kernel_name, sizeof(kernel_name), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    if (program.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL program is not specified");
        return NULL;
    }
    else if (kernel_name == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL kernel name is not specified");
        return NULL;
    }

    // Construct the context
    struct archi_context_data__opencl_kernel *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__opencl_kernel){
        .kernel = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL),
        },
        .ref_program = archi_rcpointer_own(program, ARCHI_ERROR_PARAM),
    };

    if (!context_data->ref_program.attr)
        goto failure;

    context_data->kernel_name = archi_string_copy(kernel_name);
    if (context_data->kernel_name == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate kernel name copy");
        goto failure;
    }

    // Create the kernel
    cl_int ret;
    context_data->kernel.ptr = clCreateKernel(program.ptr, kernel_name, &ret);

    if (context_data->kernel.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't create an OpenCL kernel: error %i", ret);
        goto failure;
    }

    // Extract the number of kernel arguments
    ret = clGetKernelInfo(context_data->kernel.ptr, CL_KERNEL_NUM_ARGS,
            sizeof(context_data->num_arguments), &context_data->num_arguments, NULL);
    if (ret != CL_SUCCESS)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract number of OpenCL kernel arguments: error %i", ret);
        goto failure;
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    if (context_data->kernel.ptr != NULL)
        clReleaseKernel(context_data->kernel.ptr);
    archi_rcpointer_disown(context_data->ref_program);
    free(context_data->kernel_name);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__opencl_kernel_clone)
{
    // Parse parameters
    archi_rcpointer_t source_kernel = {0};
    {
        archi_plist_param_t parsed[] = {
            {.name = "kernel",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL)}},
                .assign = {archi_plist_assign__rcpointer, &source_kernel, sizeof(source_kernel), NULL}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    if (source_kernel.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL kernel is not specified");
        return NULL;
    }

    // Construct the context
    struct archi_context_data__opencl_kernel *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__opencl_kernel){
        .kernel = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_KERNEL),
        },
        .ref_program = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_PROGRAM),
            .ref_count = source_kernel.ref_count,
        },
    };

    // Clone the kernel
    cl_int ret;
    context_data->kernel.ptr = clCloneKernel(source_kernel.ptr, &ret);

    if (context_data->kernel.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't clone an OpenCL kernel: error %i", ret);
        goto failure;
    }

    // Obtain the kernel program
    ret = clGetKernelInfo(context_data->kernel.ptr, CL_KERNEL_PROGRAM,
            sizeof(context_data->ref_program.ptr), &context_data->ref_program.ptr, NULL);

    if (ret != CL_SUCCESS)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract an OpenCL program from kernel: error %i", ret);
        goto failure;
    }

    // Obtain the kernel name
    {
        size_t name_len;
        ret = clGetKernelInfo(context_data->kernel.ptr, CL_KERNEL_FUNCTION_NAME, 0, NULL, &name_len);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract size of the name of an OpenCL kernel: error %i", ret);
            goto failure;
        }

        context_data->kernel_name = malloc(name_len);
        if (context_data->kernel_name == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate kernel name buffer (size = %zu)", name_len);
            goto failure;
        }

        ret = clGetKernelInfo(context_data->kernel.ptr, CL_KERNEL_FUNCTION_NAME, name_len, context_data->kernel_name, NULL);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract name of an OpenCL kernel: error %i", ret);
            goto failure;
        }
    }

    // Extract the number of kernel arguments
    ret = clGetKernelInfo(context_data->kernel.ptr, CL_KERNEL_NUM_ARGS,
            sizeof(context_data->num_arguments), &context_data->num_arguments, NULL);
    if (ret != CL_SUCCESS)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't extract number of OpenCL kernel arguments: error %i", ret);
        goto failure;
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    if (context_data->kernel.ptr != NULL)
        clReleaseKernel(context_data->kernel.ptr);
    free(context_data->kernel_name);
    free(context_data);

    return NULL;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__opencl_kernel)
{
    struct archi_context_data__opencl_kernel *context_data =
        (struct archi_context_data__opencl_kernel*)context;

    clReleaseKernel(context_data->kernel.ptr);
    if (context_data->ref_program.ref_count != ARCHI_CONTEXT_REF_COUNT)
        archi_rcpointer_disown(context_data->ref_program);
    free(context_data->kernel_name);
    free(context_data);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__opencl_kernel)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    struct archi_context_data__opencl_kernel *context_data =
        (struct archi_context_data__opencl_kernel*)context;

    if (ARCHI_STRING_COMPARE("program", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(context_data->ref_program);
    }
    else if (ARCHI_STRING_COMPARE("name", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_rcpointer_t value = {
            .ptr = context_data->kernel_name,
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                ARCHI_POINTER_ATTR__PDATA(strlen(context_data->kernel_name) + 1, char),
            .ref_count = ARCHI_CONTEXT_REF_COUNT,
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("num_args", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        cl_uint num_arguments = context_data->num_arguments;

        archi_rcpointer_t value = {
            .ptr = &num_arguments,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, cl_uint),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__opencl_kernel)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

    struct archi_context_data__opencl_kernel *context_data =
        (struct archi_context_data__opencl_kernel*)context;

    if (ARCHI_STRING_COMPARE("arg.local_mem_size", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__PDATA(1, size_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a size");
            return;
        }

        archi_context_slot_index_t arg_index = slot.index[0];
        if ((arg_index < 0) || ((size_t)arg_index >= context_data->num_arguments))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %u)",
                        slot.index[0], context_data->num_arguments);
            return;
        }

        size_t arg_size = *(size_t*)value.ptr;

        cl_int ret = clSetKernelArg(context_data->kernel.ptr, arg_index, arg_size, NULL);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't set OpenCL kernel argument #%lli: error %i",
                    arg_index, ret);
            return;
        }

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("arg.value", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }

        size_t length, stride;

        if (!archi_pointer_attr_unpk__pdata(value.attr,
                    &length, &stride, NULL, ARCHI_ERROR_PARAM))
            return;

        archi_context_slot_index_t arg_index = slot.index[0];
        if ((arg_index < 0) || ((size_t)arg_index >= context_data->num_arguments))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %u)",
                        slot.index[0], context_data->num_arguments);
            return;
        }

        cl_int ret = clSetKernelArg(context_data->kernel.ptr, arg_index,
                length * stride, value.ptr);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't set OpenCL kernel argument #%lli: error %i",
                    arg_index, ret);
            return;
        }

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("arg.mem_object", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__OPENCL_MEM_OBJECT)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a memory object");
            return;
        }

        archi_context_slot_index_t arg_index = slot.index[0];
        if ((arg_index < 0) || ((size_t)arg_index >= context_data->num_arguments))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %u)",
                        slot.index[0], context_data->num_arguments);
            return;
        }

        cl_int ret = clSetKernelArg(context_data->kernel.ptr, arg_index, sizeof(cl_mem), value.ptr);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't set OpenCL kernel argument #%lli: error %i",
                    arg_index, ret);
            return;
        }

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("arg.svm_ptr", ==, slot.name))
    {
        if (slot.num_indices != 1)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    archi_pointer_attr__cdata(0)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not data");
            return;
        }

        archi_context_slot_index_t arg_index = slot.index[0];
        if ((arg_index < 0) || ((size_t)arg_index >= context_data->num_arguments))
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "index (%lli) out of range [0; %u)",
                        slot.index[0], context_data->num_arguments);
            return;
        }

        cl_int ret = clSetKernelArgSVMPointer(context_data->kernel.ptr, arg_index, value.ptr);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't set OpenCL kernel argument #%lli (SVM): error %i",
                    arg_index, ret);
            return;
        }

        ARCHI_ERROR_RESET();
    }
    else if (ARCHI_STRING_COMPARE("exec_info.svm_ptrs", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        size_t length, stride;

        if (!archi_pointer_attr_unpk__pdata(value.attr,
                    &length, &stride, NULL, ARCHI_ERROR_PARAM))
            return;
        else if (stride != sizeof(void*))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not an array of data pointers");
            return;
        }

        ARCHI_POINTER_NULLIFY_EMPTY(value);

        cl_int ret = clSetKernelExecInfo(context_data->kernel.ptr, CL_KERNEL_EXEC_INFO_SVM_PTRS,
                length * stride, value.ptr);
        if (ret != CL_SUCCESS)
        {
            ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't set OpenCL kernel SVM pointers: error %i",
                    ret);
            return;
        }

        ARCHI_ERROR_RESET();
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

const archi_context_interface_t
archi_context_new_interface__opencl_kernel = {
    .init_fn = archi_context_init__opencl_kernel_new,
    .final_fn = archi_context_final__opencl_kernel,
    .eval_fn = archi_context_eval__opencl_kernel,
    .set_fn = archi_context_set__opencl_kernel,
};

const archi_context_interface_t
archi_context_clone_interface__opencl_kernel = {
    .init_fn = archi_context_init__opencl_kernel_clone,
    .final_fn = archi_context_final__opencl_kernel,
    .eval_fn = archi_context_eval__opencl_kernel,
    .set_fn = archi_context_set__opencl_kernel,
};

