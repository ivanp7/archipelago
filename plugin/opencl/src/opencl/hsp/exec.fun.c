/**
 * @file
 * @brief Hierarchical state processor states for OpenCL kernel execution.
 */

#include "archip/opencl/hsp/exec.fun.h"
#include "archip/opencl/hsp/exec.typ.h"
#include "archip/opencl/status.fun.h"
#include "archi/hsp/state.fun.h"
#include "archi/log/print.fun.h"

ARCHI_HSP_STATE_FUNCTION(archip_opencl_hsp_state_wait_for_events)
{
    archip_opencl_event_array_t *event_array = ARCHI_HSP_CURRENT_STATE().data;
    if ((event_array == NULL) || (event_array->num_events == 0) || (event_array->event == NULL))
        return;

    clWaitForEvents(event_array->num_events, event_array->event);

    for (cl_uint i = 0; i < event_array->num_events; i++)
    {
        clReleaseEvent(event_array->event[i]);
        event_array->event[i] = NULL;
    }
}

ARCHI_HSP_STATE_FUNCTION(archip_opencl_hsp_state_kernel_enqueue)
{
#define M "archip_opencl_hsp_state_kernel_enqueue"

    archip_opencl_kernel_enqueue_data_t *enqueue_data = ARCHI_HSP_CURRENT_STATE().data;
    if ((enqueue_data == NULL) || (enqueue_data->work == NULL))
        return;

    archip_opencl_work_t work = *enqueue_data->work;

    archip_opencl_event_array_t wait_list;
    if (enqueue_data->wait_list != NULL)
        wait_list = *enqueue_data->wait_list;
    else
        wait_list = (archip_opencl_event_array_t){0};

    cl_event event, *event_ptr;
    if (enqueue_data->num_event_copies > 0)
        event_ptr = &event;
    else
        event_ptr = NULL;

    cl_int ret = clEnqueueNDRangeKernel(
            enqueue_data->command_queue, enqueue_data->kernel,
            work.num_dimensions, work.global_work_offset,
            work.global_work_size, work.local_work_size,
            wait_list.num_events, wait_list.event, event_ptr);

    if (enqueue_data->name != NULL)
        archi_log_debug(M, "clEnqueueNDRangeKernel('%s') -> %s",
                enqueue_data->name, archip_opencl_error_string(ret));

    for (cl_uint i = 0; i < wait_list.num_events; i++)
    {
        clReleaseEvent(wait_list.event[i]);
        wait_list.event[i] = NULL;
    }

    if (enqueue_data->num_event_copies > 0)
    {
        for (size_t i = 0; i < enqueue_data->num_event_copies; i++)
        {
            archip_opencl_event_array_t *event_array = enqueue_data->target_event_arrays[i];

            event_array->event[enqueue_data->target_event_array_indices[i]] = event;
            clRetainEvent(event);
        }

        clReleaseEvent(event);
    }

#undef M
}

