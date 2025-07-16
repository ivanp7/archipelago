/**
 * @file
 * @brief Application context interface for data of HSP state for OpenCL kernel execution.
 */

#include "archip/opencl/ctx/exec.var.h"
#include "archip/opencl/hsp/exec.typ.h"
#include "archi/util/alloc.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), strlen()
#include <stdalign.h> // for alignof()

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
        char *name_copy = archi_copy_string(name);
        if (name_copy == NULL)
        {
            free(enqueue_data);
            free(context_data);
            return ARCHI_STATUS_ENOMEMORY;
        }

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

