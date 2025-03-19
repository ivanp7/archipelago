/**
 * @file
 * @brief Context interfaces of the plugin.
 */

#include "opencl/interface.fun.h"
#include "opencl/interface.typ.h"
#include "opencl/program.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"
#include "archi/util/print.fun.h"

#include <string.h> // for strcmp(), memcpy()
#include <CL/cl.h>

static
ARCHI_LIST_ACT_FUNC(plugin_opencl_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    plugin_opencl_context_config_t *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, PLUGIN_OPENCL_CONTEXT_CONFIG_KEY) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) ||
                (value.size != sizeof(*config)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value.ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(name, PLUGIN_OPENCL_CONTEXT_CONFIG_KEY_PLATFORM_ID) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(cl_platform_id)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->platform_id = *(cl_platform_id*)value.ptr;
        return 0;
    }
    else if (strcmp(name, PLUGIN_OPENCL_CONTEXT_CONFIG_KEY_DEVICE_IDS) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(cl_device_id)))
            return ARCHI_ERROR_CONFIG;

        config->num_devices = value.num_of;
        config->device_ids = value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(plugin_opencl_context_init)
{
    (void) metadata;

    plugin_opencl_context_config_t context_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        archi_status_t code = archi_list_traverse(&config_list, NULL, NULL,
                plugin_opencl_context_init_config, &context_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)context_config.platform_id,
        0};

    cl_int ret;

    cl_context opencl_context = clCreateContext(properties,
            context_config.num_devices, context_config.device_ids, NULL, NULL, &ret);
    if (opencl_context == NULL)
    {
        archi_log_error("plugin_opencl_context_init", "clCreateContext(#%u) failed with error %i",
                context_config.platform_id, ret);
        return ARCHI_ERROR_OPERATION;
    }

    *context = opencl_context;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(plugin_opencl_context_final)
{
    (void) metadata;

    clReleaseContext(context);
}

const archi_context_interface_t plugin_opencl_context_interface = {
    .init_fn = plugin_opencl_context_init,
    .final_fn = plugin_opencl_context_final,
};

/*****************************************************************************/

struct plugin_opencl_command_queue_config_with_context {
    cl_context context;
    plugin_opencl_command_queue_config_t config;
};

static
ARCHI_LIST_ACT_FUNC(plugin_opencl_command_queue_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    struct plugin_opencl_command_queue_config_with_context *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_CONTEXT) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->context = value.ptr;
        return 0;
    }
    else if (strcmp(name, PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) ||
                (value.size != sizeof(*config)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value.ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(name, PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_DEVICE_ID) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(cl_device_id)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->config.device_id = *(cl_device_id*)value.ptr;
        return 0;
    }
    else if (strcmp(name, PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_OOOE) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->config.out_of_order_exec = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->config.out_of_order_exec = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else if (strcmp(name, PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_PROFILING) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->config.profiling = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->config.profiling = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
#if defined(cl_khr_priority_hints)
    else if (strcmp(name, PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_PRIORITY_HINT) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(cl_queue_priority_khr)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->config.priority_hint = *(cl_queue_priority_khr*)value.ptr;
        return 0;
    }
#endif
#if defined(cl_khr_throttle_hints)
    else if (strcmp(name, PLUGIN_OPENCL_COMMAND_QUEUE_CONFIG_KEY_THROTTLE_HINT) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(cl_queue_throttle_khr)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->config.throttle_hint = *(cl_queue_throttle_khr*)value.ptr;
        return 0;
    }
#endif
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(plugin_opencl_command_queue_init)
{
    (void) metadata;

    struct plugin_opencl_command_queue_config_with_context command_queue_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        archi_status_t code = archi_list_traverse(&config_list, NULL, NULL,
                plugin_opencl_command_queue_init_config, &command_queue_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    if (command_queue_config.context == NULL)
        return ARCHI_ERROR_MISUSE;

    cl_queue_properties properties[] = {
        CL_QUEUE_PROPERTIES, (command_queue_config.config.out_of_order_exec ?
                CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE : 0) |
            (command_queue_config.config.profiling ? CL_QUEUE_PROFILING_ENABLE : 0),
#if defined(cl_khr_priority_hints)
        CL_QUEUE_PRIORITY_KHR, command_queue_config.config.priority_hint,
#endif
#if defined(cl_khr_throttle_hints)
        CL_QUEUE_THROTTLE_KHR, command_queue_config.config.throttle_hint,
#endif
    };

    cl_int ret;

    cl_command_queue opencl_command_queue = clCreateCommandQueueWithProperties(
            command_queue_config.context, command_queue_config.config.device_id,
            properties, &ret);
    if (opencl_command_queue == NULL)
    {
        archi_log_error("plugin_opencl_command_queue_init",
                "clCreateCommandQueueWithProperties(#%u) failed with error %i",
                command_queue_config.config.device_id, ret);
        return ARCHI_ERROR_OPERATION;
    }

    *context = opencl_command_queue;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(plugin_opencl_command_queue_final)
{
    (void) metadata;

    clReleaseCommandQueue(context);
}

const archi_context_interface_t plugin_opencl_command_queue_interface = {
    .init_fn = plugin_opencl_command_queue_init,
    .final_fn = plugin_opencl_command_queue_final,
};

/*****************************************************************************/

struct plugin_opencl_program_config {
    cl_context context;
    const plugin_opencl_program_binaries_t *binaries;
};

static
ARCHI_LIST_ACT_FUNC(plugin_opencl_program_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    struct plugin_opencl_program_config *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, PLUGIN_OPENCL_PROGRAM_CONFIG_KEY_CONTEXT) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->context = value.ptr;
        return 0;
    }
    else if (strcmp(name, PLUGIN_OPENCL_PROGRAM_CONFIG_KEY_BINARIES) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) || (value.num_of == 0) ||
                ((value.size != 0) && (value.size != sizeof(plugin_opencl_program_binaries_t))))
            return ARCHI_ERROR_CONFIG;

        config->binaries = value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(plugin_opencl_program_init)
{
    (void) metadata;

    archi_status_t code;

    struct plugin_opencl_program_config program_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                plugin_opencl_program_init_config, &program_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    if ((program_config.context == NULL) || (program_config.binaries == NULL))
        return ARCHI_ERROR_MISUSE;

    cl_program program = plugin_opencl_program_create(program_config.context,
            *program_config.binaries, true, &code);
    if (code != 0)
        return code;

    *context = program;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(plugin_opencl_program_final)
{
    (void) metadata;

    clReleaseProgram(context);
}

const archi_context_interface_t plugin_opencl_program_interface = {
    .init_fn = plugin_opencl_program_init,
    .final_fn = plugin_opencl_program_final,
};

/*****************************************************************************/

struct plugin_opencl_kernel_config {
    cl_program program;
    const char *kernel_name;
};

static
ARCHI_LIST_ACT_FUNC(plugin_opencl_kernel_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    struct plugin_opencl_kernel_config *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, PLUGIN_OPENCL_KERNEL_CONFIG_KEY_PROGRAM) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->program = value.ptr;
        return 0;
    }
    else if (strcmp(name, PLUGIN_OPENCL_KERNEL_CONFIG_KEY_NAME) == 0)
    {
        if ((value.type != ARCHI_VALUE_STRING) || (value.ptr == NULL) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->kernel_name = value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(plugin_opencl_kernel_init)
{
    (void) metadata;

    struct plugin_opencl_kernel_config kernel_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        archi_status_t code = archi_list_traverse(&config_list, NULL, NULL,
                plugin_opencl_kernel_init_config, &kernel_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    if ((kernel_config.program == NULL) || (kernel_config.kernel_name == NULL))
        return ARCHI_ERROR_MISUSE;

    cl_int ret;

    cl_kernel kernel = clCreateKernel(kernel_config.program, kernel_config.kernel_name, &ret);
    if (kernel == NULL)
    {
        archi_log_error("plugin_opencl_kernel_init", "clCreateKernel('%s') failed with error %i",
                kernel_config.kernel_name, ret);
        return ARCHI_ERROR_OPERATION;
    }

    *context = kernel;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(plugin_opencl_kernel_final)
{
    (void) metadata;

    clReleaseKernel(context);
}

const archi_context_interface_t plugin_opencl_kernel_interface = {
    .init_fn = plugin_opencl_kernel_init,
    .final_fn = plugin_opencl_kernel_final,
};

