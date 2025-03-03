/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Archipelago.                                         *
 *                                                                           *
 *   Archipelago is free software: you can redistribute it and/or modify it  *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Archipelago is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Operations with threads and concurrent processing.
 */

#include "archi/plugin/threads.h"
#include "archi/util/os/threads.fun.h"
#include "archi/util/os/queue.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h>
#include <string.h> // for strcmp(), memcpy()

static
ARCHI_LIST_ACT_FUNC(archi_plugin_thread_group_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    archi_thread_group_config_t *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, ARCHI_THREADS_CONFIG_KEY) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) ||
                (value.size != sizeof(*config)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value.ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(name, ARCHI_THREADS_CONFIG_KEY_NUM_THREADS) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(config->num_threads)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->num_threads = *(size_t*)value.ptr;
        return 0;
    }
    else if (strcmp(name, ARCHI_THREADS_CONFIG_KEY_BUSY_WAIT) == 0)
    {
        switch (value.type)
        {
            case ARCHI_VALUE_FALSE:
                config->busy_wait = false;
                return 0;

            case ARCHI_VALUE_TRUE:
                config->busy_wait = true;
                return 0;

            default:
                return ARCHI_ERROR_CONFIG;
        }
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_thread_group_context_init)
{
    (void) metadata;

    archi_status_t code;

    archi_thread_group_config_t thread_group_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                archi_plugin_thread_group_context_init_config, &thread_group_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    struct archi_thread_group_context *thread_group_context = archi_thread_group_start(thread_group_config, &code);
    if (code != 0)
        return code;

    *context = thread_group_context;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_thread_group_context_final)
{
    (void) metadata;

    archi_thread_group_stop(context);
}

const archi_context_interface_t archi_plugin_thread_group_context_interface = {
    .init_fn = archi_plugin_thread_group_context_init,
    .final_fn = archi_plugin_thread_group_context_final,
};

/*****************************************************************************/

static
ARCHI_LIST_ACT_FUNC(archi_plugin_queue_context_init_config)
{
    (void) position;

    archi_list_node_named_value_t *config_node = (archi_list_node_named_value_t*)node;
    archi_queue_config_t *config = data;

    const char *name = config_node->base.name;
    archi_value_t value = config_node->value;

    if (strcmp(name, ARCHI_QUEUE_CONFIG_KEY) == 0)
    {
        if ((value.type != ARCHI_VALUE_DATA) || (value.ptr == NULL) ||
                (value.size != sizeof(*config)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        memcpy(config, value.ptr, sizeof(*config));
        return 0;
    }
    else if (strcmp(name, ARCHI_QUEUE_CONFIG_KEY_CAPACITY_LOG2) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(config->capacity_log2)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->capacity_log2 = *(size_t*)value.ptr;
        return 0;
    }
    else if (strcmp(name, ARCHI_QUEUE_CONFIG_KEY_ELEMENT_ALIGNMENT_LOG2) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(config->element_alignment_log2)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->element_alignment_log2 = *(size_t*)value.ptr;
        return 0;
    }
    else if (strcmp(name, ARCHI_QUEUE_CONFIG_KEY_ELEMENT_SIZE) == 0)
    {
        if ((value.type != ARCHI_VALUE_UINT) || (value.ptr == NULL) ||
                (value.size != sizeof(config->element_size)) || (value.num_of == 0))
            return ARCHI_ERROR_CONFIG;

        config->element_size = *(size_t*)value.ptr;
        return 0;
    }
    else
        return ARCHI_ERROR_CONFIG;
}

ARCHI_CONTEXT_INIT_FUNC(archi_plugin_queue_context_init)
{
    (void) metadata;

    archi_status_t code;

    archi_queue_config_t queue_config = {0};
    if (config != NULL)
    {
        archi_list_t config_list = {.head = (archi_list_node_t*)config};
        code = archi_list_traverse(&config_list, NULL, NULL,
                archi_plugin_queue_context_init_config, &queue_config, true, 0, NULL);
        if (code != 0)
            return code;
    }

    struct archi_queue *queue_context = archi_queue_alloc(queue_config);
    if (queue_context == NULL)
        return ARCHI_ERROR_ALLOC;

    *context = queue_context;
    return 0;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_plugin_queue_context_final)
{
    (void) metadata;

    archi_queue_free(context);
}

const archi_context_interface_t archi_plugin_queue_context_interface = {
    .init_fn = archi_plugin_queue_context_init,
    .final_fn = archi_plugin_queue_context_final,
};

