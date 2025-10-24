/**
 * @file
 * @brief Application context interface for data of HSP state for OpenCL kernel execution.
 */

#include "archi/opencl/ctx/exec.var.h"
#include "archi/opencl/hsp/exec.typ.h"
#include "archipelago/base/ref_count.fun.h"
#include "archipelago/util/alloc.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp(), strlen()
#include <stdalign.h> // for alignof()

struct archi_opencl_kernel_enqueue_data_data_output_event_list;

struct archi_opencl_kernel_enqueue_data_data_output_event_list {
    struct archi_opencl_kernel_enqueue_data_data_output_event_list *next;
    archi_pointer_t event_ptr;
};

struct archi_opencl_kernel_enqueue_data_data {
    archi_pointer_t enqueue_data;

    // References
    archi_pointer_t command_queue;
    archi_pointer_t kernel;

    archi_pointer_t global_work_offset;
    archi_pointer_t global_work_size;
    archi_pointer_t local_work_size;

    archi_pointer_t wait_list;
    struct archi_opencl_kernel_enqueue_data_data_output_event_list *output_event_list;
};

ARCHI_CONTEXT_INIT_FUNC(archi_opencl_kernel_enqueue_data_init)
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

    struct archi_opencl_kernel_enqueue_data_data *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    archi_opencl_kernel_enqueue_data_t *enqueue_data = malloc(sizeof(*enqueue_data));
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

    *enqueue_data = (archi_opencl_kernel_enqueue_data_t){
        .command_queue = command_queue.ptr,
        .kernel = kernel.ptr,
        .global_work_offset = global_work_offset.ptr,
        .global_work_size = global_work_size.ptr,
        .local_work_size = local_work_size.ptr,
        .wait_list = wait_list.ptr,
        .name = name,
    };

    *context_data = (struct archi_opencl_kernel_enqueue_data_data){
        .enqueue_data = {
            .ptr = enqueue_data,
            .element = {
                .num_of = 1,
                .size = sizeof(*enqueue_data),
                .alignment = alignof(archi_opencl_kernel_enqueue_data_t),
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

ARCHI_CONTEXT_FINAL_FUNC(archi_opencl_kernel_enqueue_data_final)
{
    struct archi_opencl_kernel_enqueue_data_data *context_data =
        (struct archi_opencl_kernel_enqueue_data_data*)context;

    {
        struct archi_opencl_kernel_enqueue_data_data_output_event_list *output_event_list =
            context_data->output_event_list;
        while (output_event_list != NULL)
        {
            struct archi_opencl_kernel_enqueue_data_data_output_event_list *next =
                output_event_list->next;

            archi_reference_count_decrement(output_event_list->event_ptr.ref_count);
            free(output_event_list);

            output_event_list = next;
        }
    }

    archi_reference_count_decrement(context_data->command_queue.ref_count);
    archi_reference_count_decrement(context_data->kernel.ref_count);
    archi_reference_count_decrement(context_data->global_work_offset.ref_count);
    archi_reference_count_decrement(context_data->global_work_size.ref_count);
    archi_reference_count_decrement(context_data->local_work_size.ref_count);
    archi_reference_count_decrement(context_data->wait_list.ref_count);

    archi_opencl_kernel_enqueue_data_t *enqueue_data = context_data->enqueue_data.ptr;

    {
        archi_opencl_event_ptr_list_t *output_event_list = enqueue_data->output_event_list;
        while (output_event_list != NULL)
        {
            archi_opencl_event_ptr_list_t *next = output_event_list->next;

            free(output_event_list);

            output_event_list = next;
        }
    }

    free((char*)enqueue_data->name);
    free(enqueue_data);
    free(context_data);
}

ARCHI_CONTEXT_GET_FUNC(archi_opencl_kernel_enqueue_data_get)
{
    struct archi_opencl_kernel_enqueue_data_data *context_data =
        (struct archi_opencl_kernel_enqueue_data_data*)context;

    archi_opencl_kernel_enqueue_data_t *enqueue_data = context_data->enqueue_data.ptr;

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

ARCHI_CONTEXT_SET_FUNC(archi_opencl_kernel_enqueue_data_set)
{
    struct archi_opencl_kernel_enqueue_data_data *context_data =
        (struct archi_opencl_kernel_enqueue_data_data*)context;

    archi_opencl_kernel_enqueue_data_t *enqueue_data = context_data->enqueue_data.ptr;

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
    else if (strcmp("name", slot.name) == 0)
    {
        if (slot.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        free((char*)enqueue_data->name);

        if (value.ptr != NULL)
        {
            enqueue_data->name = archi_copy_string(value.ptr);
            if (enqueue_data->name == NULL)
                return ARCHI_STATUS_ENOMEMORY;
        }
        else
            enqueue_data->name = NULL;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

ARCHI_CONTEXT_ACT_FUNC(archi_opencl_kernel_enqueue_data_act)
{
    struct archi_opencl_kernel_enqueue_data_data *context_data =
        (struct archi_opencl_kernel_enqueue_data_data*)context;

    archi_opencl_kernel_enqueue_data_t *enqueue_data = context_data->enqueue_data.ptr;

    if (strcmp("add_output_event", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;

        archi_pointer_t output_event = {0};

        bool param_output_event_ptr_set = false;

        for (; params != NULL; params = params->next)
        {
            if (strcmp("ptr", params->name) == 0)
            {
                if (param_output_event_ptr_set)
                    continue;
                param_output_event_ptr_set = true;

                if (params->value.flags & ARCHI_POINTER_FLAG_FUNCTION)
                    return ARCHI_STATUS_EVALUE;

                output_event = params->value;
            }
            else
                return ARCHI_STATUS_EKEY;
        }

        if (output_event.ptr == NULL)
            return 0;

        archi_opencl_event_ptr_list_t *output_event_list = malloc(sizeof(*output_event_list));
        if (output_event_list == NULL)
            return ARCHI_STATUS_ENOMEMORY;

        *output_event_list = (archi_opencl_event_ptr_list_t){
            .next = enqueue_data->output_event_list,
            .event_ptr = output_event.ptr,
        };

        struct archi_opencl_kernel_enqueue_data_data_output_event_list *output_event_list_ref =
            malloc(sizeof(*output_event_list_ref));
        if (output_event_list_ref == NULL)
        {
            free(output_event_list);
            return ARCHI_STATUS_ENOMEMORY;
        }

        *output_event_list_ref = (struct archi_opencl_kernel_enqueue_data_data_output_event_list){
            .next = context_data->output_event_list,
            .event_ptr = output_event,
        };

        archi_reference_count_increment(output_event.ref_count);

        enqueue_data->output_event_list = output_event_list;
        context_data->output_event_list = output_event_list_ref;
    }
    else if (strcmp("reset_output_events", action.name) == 0)
    {
        if (action.num_indices != 0)
            return ARCHI_STATUS_EMISUSE;
        else if (params != NULL)
            return ARCHI_STATUS_EKEY;

        for (archi_opencl_event_ptr_list_t *output_event_list = enqueue_data->output_event_list;
                output_event_list != NULL;)
        {
            archi_opencl_event_ptr_list_t *next = output_event_list->next;

            free(output_event_list);

            output_event_list = next;
        }
        enqueue_data->output_event_list = NULL;

        for (struct archi_opencl_kernel_enqueue_data_data_output_event_list *output_event_list =
                context_data->output_event_list; output_event_list != NULL;)
        {
            struct archi_opencl_kernel_enqueue_data_data_output_event_list *next =
                output_event_list->next;

            archi_reference_count_decrement(output_event_list->event_ptr.ref_count);
            free(output_event_list);

            output_event_list = next;
        }
        context_data->output_event_list = NULL;
    }
    else
        return ARCHI_STATUS_EKEY;

    return 0;
}

const archi_context_interface_t archi_opencl_kernel_enqueue_data_interface = {
    .init_fn = archi_opencl_kernel_enqueue_data_init,
    .final_fn = archi_opencl_kernel_enqueue_data_final,
    .get_fn = archi_opencl_kernel_enqueue_data_get,
    .set_fn = archi_opencl_kernel_enqueue_data_set,
    .act_fn = archi_opencl_kernel_enqueue_data_act,
};

