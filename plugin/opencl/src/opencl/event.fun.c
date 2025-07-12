/**
 * @file
 * @brief Operations on OpenCL event lists.
 */

#include "archip/opencl/event.fun.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc()

archip_opencl_event_array_t*
archip_opencl_event_array_alloc(
        cl_uint num_events)
{
    archip_opencl_event_array_t *event_array = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archip_opencl_event_array_t, event, num_events));
    if (event_array == NULL)
        return NULL;

    cl_uint *num_events_ptr = (cl_uint*)&event_array->num_events;
    *num_events_ptr = num_events;

    for (cl_uint i = 0; i < num_events; i++)
        event_array->event[i] = NULL;

    return event_array;
}

void
archip_opencl_event_array_reset(
        archip_opencl_event_array_t *event_array)
{
    if (event_array == NULL)
        return;

    for (cl_uint i = 0; i < event_array->num_events; i++)
    {
        clReleaseEvent(event_array->event[i]);
        event_array->event[i] = NULL;
    }
}

