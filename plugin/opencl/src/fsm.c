/**
 * @file
 * @brief OpenCL processing states for finite state machines.
 */

#include "opencl/fsm.h"
#include "archi/fsm/state.fun.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"

#define M "plugin_opencl_fsm_state_enqueue_kernel"

ARCHI_FSM_STATE_FUNCTION(plugin_opencl_fsm_state_enqueue_kernel)
{
    plugin_opencl_fsm_state_data_enqueue_kernel_t *data =
        ARCHI_FSM_CURRENT_DATA(plugin_opencl_fsm_state_data_enqueue_kernel_t);

    if (data == NULL)
    {
        ARCHI_FSM_SET_CODE(ARCHI_ERROR_MISUSE);
        return;
    }

    cl_int err = clEnqueueNDRangeKernel(data->command_queue, data->kernel,
            data->work_dim, data->global_work_offset, data->global_work_size, data->local_work_size,
            data->num_events_in_wait_list, data->event_wait_list,
            data->event);
    if (err != CL_SUCCESS)
    {
        if (data->logging)
            archi_log_error(M, "clEnqueueNDRangeKernel() failed with error %i", err);

        ARCHI_FSM_SET_CODE(ARCHI_ERROR_OPERATION);
        return;
    }

    if ((data->event != NULL) && (data->wait_for_completion))
    {
        err = clWaitForEvents(1, data->event);

        if (err != CL_SUCCESS)
        {
            archi_log_warning(M, "clWaitForEvents() failed with error %i", err);

            ARCHI_FSM_SET_CODE(ARCHI_ERROR_OPERATION);
            return;
        }
    }
}

