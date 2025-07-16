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
            .ref_count = context->ref_count,
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
                .ref_count = context->ref_count,
                .element = {
                    .num_of = context_data->binaries.ids->num_devices,
                    .size = sizeof(context_data->binaries.ids->device_id[0]),
                    .alignment = alignof(cl_device_id),
                },
            };
        }
        else
        {
            if ((slot.index[0] < 0) || (slot.index[0] >= context_data->binaries.ids->num_devices))
                return ARCHI_STATUS_EMISUSE;

            *value = (archi_pointer_t){
                .ptr = context_data->binaries.ids->device_id[slot.index[0]],
                .ref_count = context->ref_count,
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
                .ref_count = context->ref_count,
                .element = {
                    .num_of = context_data->binaries.ids->num_devices,
                    .size = sizeof(context_data->binaries.sizes[0]),
                    .alignment = alignof(size_t),
                },
            };
        }
        else
        {
            if ((slot.index[0] < 0) || (slot.index[0] >= context_data->binaries.ids->num_devices))
                return ARCHI_STATUS_EMISUSE;

            *value = (archi_pointer_t){
                .ptr = &context_data->binaries.sizes[slot.index[0]],
                .ref_count = context->ref_count,
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
                .ref_count = context->ref_count,
                .element = {
                    .num_of = context_data->binaries.ids->num_devices,
                    .size = sizeof(context_data->binaries.contents[0]),
                    .alignment = alignof(unsigned char*),
                },
            };
        }
        else
        {
            if ((slot.index[0] < 0) || (slot.index[0] >= context_data->binaries.ids->num_devices))
                return ARCHI_STATUS_EMISUSE;

            *value = (archi_pointer_t){
                .ptr = context_data->binaries.contents[slot.index[0]],
                .ref_count = context->ref_count,
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

