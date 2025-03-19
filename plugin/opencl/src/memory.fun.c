/**
 * @file
 * @brief Memory operations.
 */

#include "opencl/memory.fun.h"
#include "opencl/memory.typ.h"

#include <CL/cl.h>

ARCHI_MEMORY_ALLOC_FUNC(plugin_opencl_memory_svm_alloc_func)
{
    plugin_opencl_memory_alloc_config_t *svm_config =
        (plugin_opencl_memory_alloc_config_t*)config;

    return clSVMAlloc(svm_config->context, svm_config->svm_mem_flags,
            svm_config->base.num_bytes, svm_config->base.alignment);
}

ARCHI_MEMORY_FREE_FUNC(plugin_opencl_memory_svm_free_func)
{
    plugin_opencl_memory_alloc_config_t *svm_config =
        (plugin_opencl_memory_alloc_config_t*)config;

    clSVMFree(svm_config->context, memory);
    return true;
}

ARCHI_MEMORY_MAP_FUNC(plugin_opencl_memory_svm_map_func)
{
    plugin_opencl_memory_map_config_t *svm_config =
        (plugin_opencl_memory_map_config_t*)config;

    return CL_SUCCESS == clEnqueueSVMMap(svm_config->command_queue, CL_TRUE,
            svm_config->map_flags, (char*)memory + svm_config->base.offset,
            svm_config->base.num_bytes, 0, NULL, NULL);
}

ARCHI_MEMORY_UNMAP_FUNC(plugin_opencl_memory_svm_unmap_func)
{
    plugin_opencl_memory_map_config_t *svm_config =
        (plugin_opencl_memory_map_config_t*)config;

    cl_int ret;

    cl_event event;
    ret = clEnqueueSVMUnmap(svm_config->command_queue,
            (char*)memory + svm_config->base.offset, 0, NULL, &event);

    if (ret == CL_SUCCESS)
        ret = clWaitForEvents(1, &event);

    return CL_SUCCESS == ret;
}

const archi_memory_interface_t plugin_opencl_memory_svm_interface = {
    .alloc_fn = plugin_opencl_memory_svm_alloc_func,
    .free_fn = plugin_opencl_memory_svm_free_func,
    .map_fn = plugin_opencl_memory_svm_map_func,
    .unmap_fn = plugin_opencl_memory_svm_unmap_func,
};

