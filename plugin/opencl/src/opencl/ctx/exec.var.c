/**
 * @file
 * @brief Application context interfaces for OpenCL kernel execution.
 */

#include "archip/opencl/ctx/exec.var.h"
#include "archip/opencl/hsp/exec.typ.h"
#include "archip/opencl/event.fun.h"
#include "archip/opencl/work.fun.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), strlen(), memcpy()
#include <stdalign.h> // for alignof()

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_event_array_init)
{
    cl_uint num_events = 0;

    bool param_num_events_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("num_events", params->name) == 0)
        {
            if (param_num_events_set)
                continue;
            param_num_events_set = true;

            if ((params->value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                    (params->value.ptr == NULL))
                return ARCHI_STATUS_EVALUE;

            num_events = *(cl_uint*)params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archip_opencl_event_array_t *event_array = archip_opencl_event_array_alloc(num_events);
    if (event_array == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *context_data = (archi_pointer_t){
        .ptr = event_array,
        .element = {
            .num_of = 1,
            .size = ARCHI_SIZEOF_FLEXIBLE(archip_opencl_event_array_t, event, num_events),
            .alignment = alignof(archip_opencl_event_array_t),
        },
    };

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_event_array_final)
{
    archip_opencl_event_array_reset(context->ptr);
    free(context->ptr);
    free(context);
}

ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_event_array_get)
{
    archip_opencl_event_array_t *event_array = context->ptr;

    if (strcmp("num_events", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = (void*)&event_array->num_events,
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(event_array->num_events),
                .alignment = alignof(cl_uint),
            },
        };
    }
    else if (strcmp("event", slot.name) == 0)
    {
        if (slot.num_indices != 1)
            return ARCHI_STATUS_EMISUSE;
        else if ((slot.index[0] < 0) || (slot.index[0] >= event_array->num_events))
            return ARCHI_STATUS_EMISUSE;

        *value = (archi_pointer_t){
            .ptr = &event_array->event[slot.index[0]],
            .ref_count = context->ref_count,
            .element = {
                .num_of = 1,
                .size = sizeof(event_array->event[0]),
                .alignment = alignof(cl_event),
            },
        };
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archip_context_opencl_event_array_interface = {
    .init_fn = archip_context_opencl_event_array_init,
    .final_fn = archip_context_opencl_event_array_final,
    .get_fn = archip_context_opencl_event_array_get,
};

///////////////////////////////////////////////////////////////////////////////

ARCHI_CONTEXT_INIT_FUNC(archip_context_opencl_work_vector_init)
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
        else if (dimensions.element.size != sizeof(((archip_opencl_work_vector_t*)NULL)->dimension[0]))
            return ARCHI_STATUS_EVALUE;
    }

    archi_pointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archip_opencl_work_vector_t *work_vector =
        archip_opencl_work_vector_alloc(num_dimensions);
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
            .size = ARCHI_SIZEOF_FLEXIBLE(archip_opencl_work_vector_t, dimension, num_dimensions),
            .alignment = alignof(archip_opencl_work_vector_t),
        },
    };

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archip_context_opencl_work_vector_final)
{
    free(context->ptr);
    free(context);
}

ARCHI_CONTEXT_GET_FUNC(archip_context_opencl_work_vector_get)
{
    archip_opencl_work_vector_t *work_vector = context->ptr;

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

ARCHI_CONTEXT_SET_FUNC(archip_context_opencl_work_vector_set)
{
    archip_opencl_work_vector_t *work_vector = context->ptr;

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

const archi_context_interface_t archip_context_opencl_work_vector_interface = {
    .init_fn = archip_context_opencl_work_vector_init,
    .final_fn = archip_context_opencl_work_vector_final,
    .get_fn = archip_context_opencl_work_vector_get,
    .set_fn = archip_context_opencl_work_vector_set,
};

///////////////////////////////////////////////////////////////////////////////

struct archip_opencl_kernel_enqueue_data_data_event_target_list;

struct archip_opencl_kernel_enqueue_data_data_event_target_list {
    struct archip_opencl_kernel_enqueue_data_data_event_target_list *next;
    archi_pointer_t event_ptr;
};

struct archip_opencl_kernel_enqueue_data_data {
    archi_pointer_t enqueue_data;

    // References
    archi_pointer_t command_queue;
    archi_pointer_t kernel;

    archi_pointer_t global_work_offset;
    archi_pointer_t global_work_size;
    archi_pointer_t local_work_size;

    archi_pointer_t wait_list;
    struct archip_opencl_kernel_enqueue_data_data_event_target_list *event_target_list;
};

ARCHI_CONTEXT_INIT_FUNC(archip_opencl_kernel_enqueue_data_init)
{
    archi_pointer_t command_queue = {0};
    archi_pointer_t kernel = {0};
    archi_pointer_t global_work_offset = {0};
    archi_pointer_t global_work_size = {0};
    archi_pointer_t local_work_size = {0};
    archi_pointer_t wait_list = {0};
    char *name = NULL;

    bool param_command_queue_set = false;
    bool param_kernel_set = false;
    bool param_global_work_offset_set = false;
    bool param_global_work_size_set = false;
    bool param_local_work_size_set = false;
    bool param_wait_list_set = false;
    bool param_name_set = false;

    for (; params != NULL; params = params->next)
    {
        if (strcmp("command_queue", params->name) == 0)
        {
            if (param_command_queue_set)
                continue;
            param_command_queue_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            command_queue = params->value;
        }
        else if (strcmp("kernel", params->name) == 0)
        {
            if (param_kernel_set)
                continue;
            param_kernel_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            kernel = params->value;
        }
        else if (strcmp("global_work_offset", params->name) == 0)
        {
            if (param_global_work_offset_set)
                continue;
            param_global_work_offset_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            global_work_offset = params->value;
        }
        else if (strcmp("global_work_size", params->name) == 0)
        {
            if (param_global_work_size_set)
                continue;
            param_global_work_size_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            global_work_size = params->value;
        }
        else if (strcmp("local_work_size", params->name) == 0)
        {
            if (param_local_work_size_set)
                continue;
            param_local_work_size_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            local_work_size = params->value;
        }
        else if (strcmp("wait_list", params->name) == 0)
        {
            if (param_wait_list_set)
                continue;
            param_wait_list_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            wait_list = params->value;
        }
        else if (strcmp("name", params->name) == 0)
        {
            if (param_name_set)
                continue;
            param_name_set = true;

            if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                return ARCHI_STATUS_EVALUE;

            name = params->value.ptr;
        }
        else
            return ARCHI_STATUS_EKEY;
    }

    struct archip_opencl_kernel_enqueue_data_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archip_opencl_kernel_enqueue_data_t *enqueue_data = malloc(sizeof(*enqueue_data));
    if (enqueue_data == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    if (name != NULL)
    {
        size_t name_len = strlen(name) + 1;
        char *name_copy = malloc(name_len);
        if (name_copy == NULL)
        {
            free(enqueue_data);
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

        memcpy(name_copy, name, name_len);
        name = name_copy;
    }

    *enqueue_data = (archip_opencl_kernel_enqueue_data_t){
        .command_queue = command_queue.ptr,
        .kernel = kernel.ptr,
        .global_work_offset = global_work_offset.ptr,
        .global_work_size = global_work_size.ptr,
        .local_work_size = local_work_size.ptr,
        .wait_list = wait_list.ptr,
        .name = name,
    };

    *context_data = (struct archip_opencl_kernel_enqueue_data_data){
        .enqueue_data = {
            .ptr = enqueue_data,
            .element = {
                .num_of = 1,
                .size = sizeof(*enqueue_data),
                .alignment = alignof(archip_opencl_kernel_enqueue_data_t),
            },
        },
        .command_queue = command_queue,
        .kernel = kernel,
        .global_work_offset = global_work_offset,
        .global_work_size = global_work_size,
        .local_work_size = local_work_size,
        .wait_list = wait_list,
    };

    archi_reference_count_increment(command_queue.ref_count);
    archi_reference_count_increment(kernel.ref_count);
    archi_reference_count_increment(global_work_offset.ref_count);
    archi_reference_count_increment(global_work_size.ref_count);
    archi_reference_count_increment(local_work_size.ref_count);
    archi_reference_count_increment(wait_list.ref_count);

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archip_opencl_kernel_enqueue_data_final)
{
    struct archip_opencl_kernel_enqueue_data_data *context_data =
        (struct archip_opencl_kernel_enqueue_data_data*)context;

    {
        struct archip_opencl_kernel_enqueue_data_data_event_target_list *event_target_list =
            context_data->event_target_list;
        while (event_target_list != NULL)
        {
            struct archip_opencl_kernel_enqueue_data_data_event_target_list *next =
                event_target_list->next;

            archi_reference_count_decrement(event_target_list->event_ptr.ref_count);
            free(event_target_list);

            event_target_list = next;
        }
    }

    archi_reference_count_decrement(context_data->command_queue.ref_count);
    archi_reference_count_decrement(context_data->kernel.ref_count);
    archi_reference_count_decrement(context_data->global_work_offset.ref_count);
    archi_reference_count_decrement(context_data->global_work_size.ref_count);
    archi_reference_count_decrement(context_data->local_work_size.ref_count);
    archi_reference_count_decrement(context_data->wait_list.ref_count);

    archip_opencl_kernel_enqueue_data_t *enqueue_data = context_data->enqueue_data.ptr;

    {
        archip_opencl_event_ptr_list_t *event_target_list = enqueue_data->event_target_list;
        while (event_target_list != NULL)
        {
            archip_opencl_event_ptr_list_t *next = event_target_list->next;

            free(event_target_list);

            event_target_list = next;
        }
    }

    free((char*)enqueue_data->name);
    free(enqueue_data);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archip_opencl_kernel_enqueue_data_get)
{
    struct archip_opencl_kernel_enqueue_data_data *context_data =
        (struct archip_opencl_kernel_enqueue_data_data*)context;

    archip_opencl_kernel_enqueue_data_t *enqueue_data = context_data->enqueue_data.ptr;

    if (strcmp("command_queue", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->command_queue;
    }
    else if (strcmp("kernel", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->kernel;
    }
    else if (strcmp("global_work_offset", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->global_work_offset;
    }
    else if (strcmp("global_work_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->global_work_size;
    }
    else if (strcmp("local_work_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->local_work_size;
    }
    else if (strcmp("wait_list", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        *value = context_data->wait_list;
    }
    else if (strcmp("name", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        if (enqueue_data->name != NULL)
            *value = (archi_pointer_t){
                .ptr = (void*)enqueue_data->name,
                .ref_count = context->ref_count,
                .element = {
                    .num_of = strlen(enqueue_data->name) + 1,
                    .size = 1,
                    .alignment = 1,
                },
            };
        else
            *value = (archi_pointer_t){0};
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_SET_FUNC(archip_opencl_kernel_enqueue_data_set)
{
    struct archip_opencl_kernel_enqueue_data_data *context_data =
        (struct archip_opencl_kernel_enqueue_data_data*)context;

    archip_opencl_kernel_enqueue_data_t *enqueue_data = context_data->enqueue_data.ptr;

    if (strcmp("command_queue", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->command_queue.ref_count);

        enqueue_data->command_queue = value.ptr;
        context_data->command_queue = value;
    }
    else if (strcmp("kernel", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->kernel.ref_count);

        enqueue_data->kernel = value.ptr;
        context_data->kernel = value;
    }
    else if (strcmp("global_work_offset", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->global_work_offset.ref_count);

        enqueue_data->global_work_offset = value.ptr;
        context_data->global_work_offset = value;
    }
    else if (strcmp("global_work_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->global_work_size.ref_count);

        enqueue_data->global_work_size = value.ptr;
        context_data->global_work_size = value;
    }
    else if (strcmp("local_work_size", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->local_work_size.ref_count);

        enqueue_data->local_work_size = value.ptr;
        context_data->local_work_size = value;
    }
    else if (strcmp("wait_list", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archi_reference_count_increment(value.ref_count);
        archi_reference_count_decrement(context_data->wait_list.ref_count);

        enqueue_data->wait_list = value.ptr;
        context_data->wait_list = value;
    }
    else if (strcmp("event_target_ptr", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        archip_opencl_event_ptr_list_t *event_target_list = malloc(sizeof(*event_target_list));
        if (event_target_list == NULL)
            return ARCHI_STATUS_ENOMEMORY;

        *event_target_list = (archip_opencl_event_ptr_list_t){
            .next = enqueue_data->event_target_list,
            .event_ptr = value.ptr,
        };

        struct archip_opencl_kernel_enqueue_data_data_event_target_list *event_target_list_ref =
            malloc(sizeof(*event_target_list_ref));
        if (event_target_list_ref == NULL)
        {
            free(event_target_list);
            return ARCHI_STATUS_ENOMEMORY;
        }

        *event_target_list_ref = (struct archip_opencl_kernel_enqueue_data_data_event_target_list){
            .next = context_data->event_target_list,
            .event_ptr = value,
        };

        archi_reference_count_increment(value.ref_count);

        enqueue_data->event_target_list = event_target_list;
        context_data->event_target_list = event_target_list_ref;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archip_opencl_kernel_enqueue_data_interface = {
    .init_fn = archip_opencl_kernel_enqueue_data_init,
    .final_fn = archip_opencl_kernel_enqueue_data_final,
    .get_fn = archip_opencl_kernel_enqueue_data_get,
    .set_fn = archip_opencl_kernel_enqueue_data_set,
};

