/**
 * @file
 * @brief Context interfaces of the plugin.
 */

#pragma once
#ifndef _PLUGIN_OPENCL_INTERFACE_FUN_H_
#define _PLUGIN_OPENCL_INTERFACE_FUN_H_

#include "archi/app/context.typ.h"

ARCHI_CONTEXT_INIT_FUNC(plugin_opencl_context_init);   ///< OpenCL context initialization function.
ARCHI_CONTEXT_FINAL_FUNC(plugin_opencl_context_final); ///< OpenCL context finalization function.

extern
const archi_context_interface_t plugin_opencl_context_interface; ///< OpenCL context interface functions.

/**
 * @brief Key of OpenCL context interface.
 */
#define PORT_OPENCL_CONTEXT_INTERFACE_KEY "cl_context"

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(plugin_opencl_command_queue_init);   ///< OpenCL command queue initialization function.
ARCHI_CONTEXT_FINAL_FUNC(plugin_opencl_command_queue_final); ///< OpenCL command queue finalization function.

extern
const archi_context_interface_t plugin_opencl_command_queue_interface; ///< OpenCL command queue interface functions.

/**
 * @brief Key of OpenCL command queue interface.
 */
#define PORT_OPENCL_COMMAND_QUEUE_INTERFACE "cl_command_queue"

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(plugin_opencl_program_init);   ///< OpenCL program initialization function.
ARCHI_CONTEXT_FINAL_FUNC(plugin_opencl_program_final); ///< OpenCL program finalization function.

extern
const archi_context_interface_t plugin_opencl_program_interface; ///< OpenCL program interface functions.

/**
 * @brief Key of OpenCL program interface.
 */
#define PORT_OPENCL_PROGRAM_INTERFACE "cl_program"

/*****************************************************************************/

ARCHI_CONTEXT_INIT_FUNC(plugin_opencl_kernel_init);   ///< OpenCL kernel initialization function.
ARCHI_CONTEXT_FINAL_FUNC(plugin_opencl_kernel_final); ///< OpenCL kernel finalization function.

extern
const archi_context_interface_t plugin_opencl_kernel_interface; ///< OpenCL kernel interface functions.

/**
 * @brief Key of OpenCL kernel interface.
 */
#define PORT_OPENCL_KERNEL_INTERFACE "cl_kernel"

#endif // _PLUGIN_OPENCL_INTERFACE_FUN_H_

