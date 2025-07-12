/**
 * @file
 * @brief Hierarchical state processor states for OpenCL kernel execution.
 */

#include "archip/opencl/hsp/exec.fun.h"
#include "archip/opencl/hsp/exec.typ.h"
#include "archip/opencl/event.fun.h"
#include "archip/opencl/status.fun.h"
#include "archi/hsp/state.fun.h"
#include "archi/log/print.fun.h"

ARCHI_HSP_STATE_FUNCTION(archip_opencl_hsp_state_wait_for_events)
{
#define M "archip_opencl_hsp_state_wait_for_events"

    archip_opencl_event_array_t *event_array = ARCHI_HSP_CURRENT_STATE().data;
    if ((event_array == NULL) || (event_array->num_events == 0))
        return;

    cl_int ret = clWaitForEvents(event_array->num_events, event_array->event);

    if (ret != CL_SUCCESS)
        archi_log_error(M, "clWaitForEvents(<%u events>) -> %s",
                event_array->num_events, archip_opencl_error_string(ret));

    archip_opencl_event_array_reset(event_array);

#undef M
}

ARCHI_HSP_STATE_FUNCTION(archip_opencl_hsp_state_kernel_enqueue)
{
#define M "archip_opencl_hsp_state_kernel_enqueue"

    archip_opencl_kernel_enqueue_data_t *enqueue_data = ARCHI_HSP_CURRENT_STATE().data;
    if ((enqueue_data == NULL) || (enqueue_data->global_work_size == NULL))
        return;

    cl_uint num_work_dimensions = enqueue_data->global_work_size->num_dimensions;
    const size_t *global_work_size = enqueue_data->global_work_size->dimension;

    const size_t *global_work_offset = NULL;
    const size_t *local_work_size = NULL;

    if (enqueue_data->global_work_offset != NULL)
    {
        if (enqueue_data->global_work_offset->num_dimensions != num_work_dimensions)
        {
            archi_log_error(M, "Incorrect number of dimensions (%u) in global work offset vector",
                    enqueue_data->global_work_offset->num_dimensions);
            return;
        }

        global_work_offset = enqueue_data->global_work_offset->dimension;
    }

    if (enqueue_data->local_work_size != NULL)
    {
        if (enqueue_data->local_work_size->num_dimensions != num_work_dimensions)
        {
            archi_log_error(M, "Incorrect number of dimensions (%u) in local work size vector",
                    enqueue_data->local_work_size->num_dimensions);
            return;
        }

        local_work_size = enqueue_data->local_work_size->dimension;
    }

    cl_uint num_wait_events = 0;
    cl_event *wait_events = NULL;

    if (enqueue_data->wait_list != NULL)
    {
        num_wait_events = enqueue_data->wait_list->num_events;
        wait_events = enqueue_data->wait_list->event;
    }

    cl_event event;

    cl_int ret = clEnqueueNDRangeKernel(
            enqueue_data->command_queue, enqueue_data->kernel,
            num_work_dimensions, global_work_offset,
            global_work_size, local_work_size,
            num_wait_events, wait_events,
            (enqueue_data->event_target_list != NULL) ? &event : NULL);

    if (ret != CL_SUCCESS)
    {
        if (enqueue_data->name != NULL)
            archi_log_error(M, "clEnqueueNDRangeKernel('%s') -> %s",
                    enqueue_data->name, archip_opencl_error_string(ret));
        else
            archi_log_error(M, "clEnqueueNDRangeKernel() -> %s",
                    archip_opencl_error_string(ret));
    }

    archip_opencl_event_array_reset(enqueue_data->wait_list);

    if (enqueue_data->event_target_list != NULL)
    {
        for (archip_opencl_event_ptr_list_t *node = enqueue_data->event_target_list;
                node != NULL; node = node->next)
        {
            if (node->event_ptr == NULL)
                continue;

            *node->event_ptr = event;
            clRetainEvent(event);
        }

        clReleaseEvent(event);
    }

#undef M
}

