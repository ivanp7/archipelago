/**
 * @file
 * @brief Memory interface for OpenCL memory.
 */

#include "archip/opencl/mem/svm.var.h"
#include "archip/opencl/mem/svm.typ.h"

#include <stdlib.h> // for malloc(), free()

struct archip_opencl_svm_allocation {
    void *memory;

    cl_context context;
    cl_command_queue command_queue;
};

ARCHI_MEMORY_ALLOC_FUNC(archip_opencl_svm_alloc)
{
    archip_opencl_svm_alloc_data_t *svm_alloc_data = alloc_data;
    if (svm_alloc_data == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    struct archip_opencl_svm_allocation *svm_allocation = malloc(sizeof(*svm_allocation));
    if (svm_allocation == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return NULL;
    }

    *svm_allocation = (struct archip_opencl_svm_allocation){
        .memory = clSVMAlloc(svm_alloc_data->context, svm_alloc_data->mem_flags,
                num_bytes, alignment),
        .context = svm_alloc_data->context,
    };

    if (svm_allocation->memory == NULL)
    {
        free(svm_allocation);

        if (code != NULL)
            *code = ARCHI_STATUS_ERESOURCE;

        return NULL;

    }

    if (code != NULL)
        *code = 0;

    return svm_allocation;
}

ARCHI_MEMORY_FREE_FUNC(archip_opencl_svm_free)
{
    struct archip_opencl_svm_allocation *svm_allocation = allocation;

    clSVMFree(svm_allocation->context, svm_allocation->memory);
    free(svm_allocation);
}

ARCHI_MEMORY_MAP_FUNC(archip_opencl_svm_map)
{
    (void) for_writing;

    struct archip_opencl_svm_allocation *svm_allocation = allocation;

    archip_opencl_svm_map_data_t *svm_map_data = map_data;
    if (svm_map_data == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    cl_int ret = clEnqueueSVMMap(svm_map_data->command_queue, CL_TRUE,
            svm_map_data->map_flags, (char*)svm_allocation->memory + offset,
            num_bytes, 0, NULL, NULL);

    if (ret != CL_SUCCESS)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ERESOURCE;

        return NULL;
    }

    svm_allocation->command_queue = svm_map_data->command_queue;

    if (code != NULL)
        *code = 0;

    return (char*)svm_allocation->memory + offset;
}

ARCHI_MEMORY_UNMAP_FUNC(archip_opencl_svm_unmap)
{
    struct archip_opencl_svm_allocation *svm_allocation = allocation;

    cl_event event;
    cl_int ret = clEnqueueSVMUnmap(svm_allocation->command_queue,
            mapping, 0, NULL, &event);

    if (ret == CL_SUCCESS)
        clWaitForEvents(1, &event);

    svm_allocation->command_queue = NULL;
}

const archi_memory_interface_t archip_opencl_svm_interface = {
    .alloc_fn = archip_opencl_svm_alloc,
    .free_fn = archip_opencl_svm_free,
    .map_fn = archip_opencl_svm_map,
    .unmap_fn = archip_opencl_svm_unmap,
};

