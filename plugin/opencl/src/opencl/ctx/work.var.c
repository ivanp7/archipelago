/**
 * @file
 * @brief Application context interfaces for OpenCL work description.
 */

#include "archi/opencl/ctx/work.var.h"
#include "archi/opencl/api/work.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), memcpy()
#include <stdalign.h> // for alignof()

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_work_vector_init)
{
    cl_uint num_dimensions = 0;
    archi_pointer_t dimensions = {0};

    bool param_num_dimensions_set = false;
    bool param_dimensions_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("num_dimensions", params->name) == 0)
        {
            if (param_num_dimensions_set)
                continue;
            param_num_dimensions_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            num_dimensions = *(cl_uint*)params->value.ptr;
        }
        else if (strcmp("dimensions", params->name) == 0)
        {
            if (param_dimensions_set)
                continue;
            param_dimensions_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            dimensions = params->value;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    if (!param_num_dimensions_set)
        num_dimensions = dimensions.element.num_of;

    if (param_dimensions_set)
    {
        if (num_dimensions != dimensions.element.num_of)
            return ARCHI_STATUS_EVALUE;
        else if (dimensions.element.size != sizeof(((archi_opencl_work_vector_t*)NULL)->dimension[0]))
            return ARCHI_STATUS_EVALUE;
    }

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_opencl_work_vector_t *work_vector =
        archi_opencl_work_vector_alloc(num_dimensions);
    if (work_vector == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    if (dimensions.ptr != NULL)
        memcpy(work_vector->dimension, dimensions.ptr,
                sizeof(work_vector->dimension[0]) * num_dimensions);

    *context_data = (archi_pointer_t){
        .ptr = work_vector,
        .element = {
            .num_of = 1,
            .size = ARCHI_SIZEOF_FLEXIBLE(archi_opencl_work_vector_t, dimension, num_dimensions),
            .alignment = alignof(archi_opencl_work_vector_t),
        },
    };

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_work_vector_final)
{
    free(context->ptr);
    free(context);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_work_vector_get)
{
    archi_opencl_work_vector_t *work_vector = context->ptr;

    if (strcmp("num_dimensions", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = (void*)&work_vector->num_dimensions,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(work_vector->num_dimensions),
                .alignment = alignof(cl_uint),
            },
        };
    }
    else if (strcmp("dimension", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || (slot.index[0] >= work_vector->num_dimensions))
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &work_vector->dimension[slot.index[0]],
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(work_vector->dimension[0]),
                .alignment = alignof(size_t),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archi_context_opencl_work_vector_set)
{
    archi_opencl_work_vector_t *work_vector = context->ptr;

    if (strcmp("dimension", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || (slot.index[0] >= work_vector->num_dimensions))
            return ARCHI_STATUS_EMISUSE;
        else if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) || (value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        work_vector->dimension[slot.index[0]] = *(size_t*)value.ptr;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_context_opencl_work_vector_interface = {
    .init_fn = archi_context_opencl_work_vector_init,
    .final_fn = archi_context_opencl_work_vector_final,
    .get_fn = archi_context_opencl_work_vector_get,
    .set_fn = archi_context_opencl_work_vector_set,
};

