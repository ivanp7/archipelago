/**
 * @file
 * @brief OpenCL processing states for finite state machines.
 */

#pragma once
#ifndef _PLUGIN_OPENCL_STATE_FUN_H_
#define _PLUGIN_OPENCL_STATE_FUN_H_

#include "archi/fsm/state.typ.h"

/**
 * @brief Enqueue an OpenCL kernel for execution.
 */
ARCHI_FSM_STATE_FUNCTION(plugin_opencl_fsm_state_enqueue_kernel);

#endif // _PLUGIN_OPENCL_STATE_FUN_H_

