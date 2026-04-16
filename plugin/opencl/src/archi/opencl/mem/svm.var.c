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
 * @brief Memory interface for OpenCL memory.
 */

#include "archi/opencl/mem/svm.var.h"
#include "archi/opencl/mem/svm.typ.h"
#include "archi/opencl/api/tag.def.h"

#include <stdlib.h> // for malloc(), free()


static
ARCHI_MEMORY_ALLOC_FUNC(archi_memory_alloc__opencl_svm)
{
    archi_memory_alloc_data__opencl_svm_t *svm_alloc_data = alloc_data;
    if (svm_alloc_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL SVM allocation data is NULL");
        return (archi_memory_alloc_info_t){0};
    }

    void *allocation = clSVMAlloc(svm_alloc_data->context,
            svm_alloc_data->mem_flags, num_bytes, alignment);
    if (allocation == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate OpenCL SVM (%zu bytes, alignment = %#zx)",
                num_bytes, alignment);
        return (archi_memory_alloc_info_t){0};
    }

    clRetainContext(svm_alloc_data->context);

    bool writable = (svm_alloc_data->mem_flags & CL_MEM_READ_WRITE) ||
        (svm_alloc_data->mem_flags & CL_MEM_WRITE_ONLY);

    ARCHI_ERROR_RESET();
    return (archi_memory_alloc_info_t){
        .allocation = {.ptr = allocation, .tag = ARCHI_POINTER_DATA_TAG__OPENCL_SVM, .writable = writable},
        .metadata = svm_alloc_data->context,
    };
}

static
ARCHI_MEMORY_FREE_FUNC(archi_memory_free__opencl_svm)
{
    cl_context context = alloc_info.metadata;

    clSVMFree(context, alloc_info.allocation.ptr);
    clReleaseContext(context);
}

static
ARCHI_MEMORY_MAP_FUNC(archi_memory_map__opencl_svm)
{
    archi_memory_map_data__opencl_svm_t *svm_map_data = map_data;
    if (svm_map_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "OpenCL SVM mapping data is NULL");
        return (archi_memory_map_info_t){0};
    }

    cl_int ret = clEnqueueSVMMap(svm_map_data->command_queue, CL_TRUE,
            svm_map_data->map_flags, (char*)alloc_info.allocation.ptr + offset,
            num_bytes, 0, NULL, NULL);

    if (ret != CL_SUCCESS)
    {
        ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't map OpenCL SVM: error %i", ret);
        return (archi_memory_map_info_t){0};
    }

    clRetainCommandQueue(svm_map_data->command_queue);

    bool writable = (svm_map_data->map_flags & CL_MAP_WRITE) ||
        (svm_map_data->map_flags & CL_MAP_WRITE_INVALIDATE_REGION);

    ARCHI_ERROR_RESET();
    return (archi_memory_map_info_t){
        .mapping = {.ptr = (char*)alloc_info.allocation.ptr + offset, .writable = writable},
        .metadata = svm_map_data->command_queue,
    };
}

static
ARCHI_MEMORY_UNMAP_FUNC(archi_memory_unmap__opencl_svm)
{
    (void) alloc_info;

    cl_command_queue command_queue = map_info.metadata;

    cl_event event;
    cl_int ret = clEnqueueSVMUnmap(command_queue, map_info.mapping.ptr, 0, NULL, &event);

    if (ret == CL_SUCCESS)
    {
        clWaitForEvents(1, &event);
        clReleaseEvent(event);
    }
    // else
    //     ARCHI_ERROR_SET(ARCHI__ESYSTEM, "couldn't unmap OpenCL SVM: error %i", ret);

    clReleaseCommandQueue(command_queue);
}

const archi_memory_interface_t
archi_memory_interface__opencl_svm = {
    .alloc_data_tag = ARCHI_POINTER_DATA_TAG__OPENCL_SVM_ALLOC_DATA,
    .map_data_tag = ARCHI_POINTER_DATA_TAG__OPENCL_SVM_MAP_DATA,
    .alloc_fn = archi_memory_alloc__opencl_svm,
    .free_fn = archi_memory_free__opencl_svm,
    .map_fn = archi_memory_map__opencl_svm,
    .unmap_fn = archi_memory_unmap__opencl_svm,
};

