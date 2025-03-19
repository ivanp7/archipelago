/**
 * @file
 * @brief Context interfaces of the plugin.
 */

#pragma once
#ifndef _PLUGIN_OPENCL_INTERFACE_TYP_H_
#define _PLUGIN_OPENCL_INTERFACE_TYP_H_

#include <CL/cl.h>
#include <stdbool.h>

/**
 * @brief Parameters for OpenCL context creation.
 */
typedef struct plugin_opencl_context_config {
    cl_platform_id platform_id; ///< OpenCL platform identifier.

    cl_uint num_devices;        ///< Number of utilized OpenCL devices on the platform.
    cl_device_id *device_ids;   ///< Array of OpenCL device identifiers.
} plugin_opencl_context_config_t;

/**
 * @brief OpenCL context configuration key for the whole configuration structure.
 */
#define PLUGIN_OPENCL_CONTEXT_CONFIG_KEY "config"

/**
 * @brief OpenCL context configuration key -- platform ID.
 */
#define PLUGIN_OPENCL_CONTEXT_CONFIG_KEY_PLATFORM_ID "platform_id"

/**
 * @brief OpenCL context configuration key -- array of device IDs.
 */
#define PLUGIN_OPENCL_CONTEXT_CONFIG_KEY_DEVICE_IDS "device_ids"

/*****************************************************************************/

/**
 * @brief Parameters for OpenCL command queue creation.
 */
typedef struct plugin_opencl_command_queue_config {
    cl_device_id device_id; ///< OpenCL device identifier.

    bool out_of_order_exec; ///< Whether out-of-order execution of commands is enabled.
    bool profiling;         ///< Whether profiling of commands is enabled.

#if defined(cl_khr_priority_hints)
    cl_queue_priority_khr priority_hint; ///< Priority hint for command queue.
#endif
#if defined(cl_khr_throttle_hints)
    cl_queue_throttle_khr throttle_hint; ///< Throttle hint for command queue.
#endif
} plugin_opencl_command_queue_config_t;

/**
 * @brief OpenCL command queue configuration key -- context.
 */
#define PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_CONTEXT "context"

/**
 * @brief OpenCL command queue configuration key for the whole configuration structure.
 */
#define PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY "config"

/**
 * @brief OpenCL command queue configuration key -- device ID.
 */
#define PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_DEVICE_ID "device_id"

/**
 * @brief OpenCL command queue configuration key -- whether out-of-order execution of commands is enabled.
 */
#define PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_OOOE "out_of_order_exec"

/**
 * @brief OpenCL command queue configuration key -- whether profiling of commands is enabled.
 */
#define PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_PROFILING "profiling"

/**
 * @brief OpenCL command queue configuration key -- priority hint for command queue.
 */
#define PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_PRIORITY_HINT "priority_hint"

/**
 * @brief OpenCL command queue configuration key -- throttle hint for command queue.
 */
#define PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_THROTTLE_HINT "throttle_hint"

/*****************************************************************************/

/**
 * @brief OpenCL program configuration key -- context.
 */
#define PLUGIN_OPENCL_PROGRAM_CONFIG_KEY_CONTEXT "context"

/**
 * @brief OpenCL program configuration key -- program binaries.
 */
#define PLUGIN_OPENCL_PROGRAM_CONFIG_KEY_BINARIES "binaries"

/*****************************************************************************/

/**
 * @brief OpenCL kernel configuration key -- program.
 */
#define PLUGIN_OPENCL_KERNEL_CONFIG_KEY_PROGRAM "program"

/**
 * @brief OpenCL kernel configuration key -- kernel name.
 */
#define PLUGIN_OPENCL_KERNEL_CONFIG_KEY_NAME "name"

#endif // _PLUGIN_OPENCL_INTERFACE_TYP_H_

