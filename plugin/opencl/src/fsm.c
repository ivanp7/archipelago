/**
 * @file
 * @brief OpenCL processing states for finite state machines.
 */

#include "opencl/fsm.h"
#include "archi/fsm/state.fun.h"

#define M "plugin_opencl_fsm_state_enqueue_kernel"

ARCHI_FSM_STATE_FUNCTION(plugin_opencl_fsm_state_enqueue_kernel)
{
    plugin_opencl_fsm_state_data_enqueue_kernel_t *data =
        ARCHI_FSM_CURRENT_DATA(plugin_opencl_fsm_state_data_enqueue_kernel_t*);

    if (data == NULL)
        return;

    data->ret = clEnqueueNDRangeKernel(data->command_queue, data->kernel,
            data->work_dim, data->global_work_offset, data->global_work_size, data->local_work_size,
            data->num_events_in_wait_list, data->event_wait_list,
            data->event);

    if ((data->ret == CL_SUCCESS) && (data->wait_for_completion) && (data->event != NULL))
        data->ret = clWaitForEvents(1, data->event);
}

