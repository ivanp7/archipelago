/**
 * @file
 * @brief Application context interfaces for OpenCL event arrays.
 */

#include "archi/opencl/ctx/event.var.h"
#include "archi/opencl/api/event.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()
#include <stdalign.h> // for alignof()

ARCHI_CONTEXT_INIT_FUNC(archi_context_opencl_event_array_init)
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

    archi_opencl_event_array_t *event_array = archi_opencl_event_array_alloc(num_events);
    if (event_array == NULL)
    {
        free(context_data);
        return ARCHI_STATUS_ENOMEMORY;
    }

    *context_data = (archi_pointer_t){
        .ptr = event_array,
        .element = {
            .num_of = 1,
            .size = ARCHI_SIZEOF_FLEXIBLE(archi_opencl_event_array_t, event, num_events),
            .alignment = alignof(archi_opencl_event_array_t),
        },
    };

    *context = (archi_pointer_t*)context_data;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_opencl_event_array_final)
{
    archi_opencl_event_array_reset(context->ptr);
    free(context->ptr);
    free(context);
}

ARCHI_CONTEXT_GET_FUNC(archi_context_opencl_event_array_get)
{
    archi_opencl_event_array_t *event_array = context->ptr;

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

const archi_context_interface_t archi_context_opencl_event_array_interface = {
    .init_fn = archi_context_opencl_event_array_init,
    .final_fn = archi_context_opencl_event_array_final,
    .get_fn = archi_context_opencl_event_array_get,
};

