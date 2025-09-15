/**
 * @file
 * @brief Memory interface for OpenCL memory.
 */

#include "archi/opencl/mem/svm.var.h"
#include "archi/opencl/mem/svm.typ.h"
#include "archipelago/log/print.fun.h"

#include <stdlib.h> // for malloc(), free()

struct archi_opencl_svm_metadata {
    cl_context context;
    cl_command_queue command_queue;
};

ARCHI_MEMORY_ALLOC_FUNC(archi_opencl_svm_alloc)
{
    archi_opencl_svm_alloc_data_t *svm_alloc_data = alloc_data;
    if (svm_alloc_data == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return (archi_memory_alloc_info_t){0};
    }

    struct archi_opencl_svm_metadata *svm_metadata = malloc(sizeof(*svm_metadata));
    if (svm_metadata == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return (archi_memory_alloc_info_t){0};
    }

    *svm_metadata = (struct archi_opencl_svm_metadata){
        .context = svm_alloc_data->context,
    };

    archi_memory_alloc_info_t alloc_info = {
        .allocation = clSVMAlloc(svm_alloc_data->context, svm_alloc_data->mem_flags,
                num_bytes, alignment),
        .metadata = svm_metadata,
    };

    if (alloc_info.allocation == NULL)
    {
        free(svm_metadata);

        if (code != NULL)
            *code = ARCHI_STATUS_ERESOURCE;

        return (archi_memory_alloc_info_t){0};
    }

    clRetainContext(svm_metadata->context);

    if (code != NULL)
        *code = 0;

    return alloc_info;
}

ARCHI_MEMORY_FREE_FUNC(archi_opencl_svm_free)
{
    struct archi_opencl_svm_metadata *svm_metadata = alloc_info.metadata;

    clSVMFree(svm_metadata->context, alloc_info.allocation);
    clReleaseContext(svm_metadata->context);
    free(svm_metadata);
}

ARCHI_MEMORY_MAP_FUNC(archi_opencl_svm_map)
{
    (void) for_writing;

    struct archi_opencl_svm_metadata *svm_metadata = alloc_info.metadata;

    archi_opencl_svm_map_data_t *svm_map_data = map_data;
    if (svm_map_data == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    cl_int ret = clEnqueueSVMMap(svm_map_data->command_queue, CL_TRUE,
            svm_map_data->map_flags, (char*)alloc_info.allocation + offset,
            num_bytes, 0, NULL, NULL);

    if (ret != CL_SUCCESS)
    {
        archi_log_error("archi_opencl_svm_map", "clEnqueueSVMMap() failed with error %i", ret);

        if (code != NULL)
            *code = ARCHI_STATUS_ERESOURCE;

        return NULL;
    }

    svm_metadata->command_queue = svm_map_data->command_queue;
    clRetainCommandQueue(svm_metadata->command_queue);

    if (code != NULL)
        *code = 0;

    return (char*)alloc_info.allocation + offset;
}

ARCHI_MEMORY_UNMAP_FUNC(archi_opencl_svm_unmap)
{
    struct archi_opencl_svm_metadata *svm_metadata = alloc_info.metadata;

    cl_event event;
    cl_int ret = clEnqueueSVMUnmap(svm_metadata->command_queue,
            mapping, 0, NULL, &event);

    if (ret == CL_SUCCESS)
        clWaitForEvents(1, &event);
    else
        archi_log_error("archi_opencl_svm_unmap", "clEnqueueSVMUnmap() failed with error %i", ret);

    clReleaseCommandQueue(svm_metadata->command_queue);
    svm_metadata->command_queue = NULL;
}

const archi_memory_interface_t archi_opencl_svm_interface = {
    .alloc_fn = archi_opencl_svm_alloc,
    .free_fn = archi_opencl_svm_free,
    .map_fn = archi_opencl_svm_map,
    .unmap_fn = archi_opencl_svm_unmap,
};

