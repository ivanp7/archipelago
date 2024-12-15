/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Applications implementation.
 */

#include "archi/app.h"
#include "archi/util/error.def.h"
#include "archi/util/list.fun.h"
#include "archi/util/os/lib.fun.h"

#include <stdio.h> // for snprintf()
#include <stdlib.h> // for malloc() and free()
#include <string.h> // for strlen(), memcpy()

#define _STR(x) #x
#define STRINGIFY(x) _STR(x)

struct archi_app_lists {
    archi_list_t *plugins;
    archi_list_t *vtables;
    archi_list_t *contexts;
};

char*
archi_application_vtable_alias_alloc(
        const char *plugin_alias,
        const char *vtable_symbol)
{
    if (plugin_alias == NULL)
        plugin_alias = "";

    if (vtable_symbol == NULL)
        vtable_symbol = "";

    size_t plugin_alias_len = strlen(plugin_alias);
    size_t vtable_symbol_len = strlen(vtable_symbol);

    size_t length = plugin_alias_len + vtable_symbol_len;
    if (length > 0)
        length += strlen(ARCHI_APP_VTABLE_ALIAS_SEPARATOR);

    char *str = malloc(length + 1);
    if (str == NULL)
        return NULL;

    if (length > 0)
    {
        char *ptr = str;

        memcpy(ptr, plugin_alias, plugin_alias_len);
        ptr += plugin_alias_len;

        memcpy(ptr, ARCHI_APP_VTABLE_ALIAS_SEPARATOR,
                strlen(ARCHI_APP_VTABLE_ALIAS_SEPARATOR));
        ptr += strlen(ARCHI_APP_VTABLE_ALIAS_SEPARATOR);

        memcpy(ptr, vtable_symbol, vtable_symbol_len);
    }

    str[length] = '\0';
    return str;
}

/*****************************************************************************/

archi_status_t
archi_application_load_plugin(
        archi_app_plugin_instance_t **node,
        const char *alias,
        const char *pathname)
{
    if ((node == NULL) || (alias == NULL) || (pathname == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    // Allocate a node
    archi_app_plugin_instance_t *plugin_node = malloc(sizeof(*plugin_node));
    if (plugin_node == NULL)
        return ARCHI_ERROR_ALLOC;

    *plugin_node = (archi_app_plugin_instance_t){0};

    // Copy plugin alias name string
    {
        size_t alias_len = strlen(alias);

        char *alias_name = malloc(alias_len + 1);
        if (alias_name == NULL)
        {
            code = ARCHI_ERROR_ALLOC;
            goto failure;
        }

        memcpy(alias_name, alias, alias_len + 1);

        plugin_node->base.name = alias_name;
    }

    // Load the plugin library
    plugin_node->handle = archi_library_load(pathname, true, false);
    if (plugin_node->handle == NULL)
    {
        code = ARCHI_ERROR_LOAD;
        goto failure;
    }

    *node = plugin_node;
    return 0;

failure:
    archi_application_unload_plugin(plugin_node);
    return code;
}

void
archi_application_unload_plugin(
        archi_app_plugin_instance_t *node)
{
    if (node == NULL)
        return;

    archi_library_unload(node->handle);
    archi_list_act_func_free_named((archi_list_node_t*)node, NULL);
}

archi_status_t
archi_application_get_vtable(
        archi_app_vtable_instance_t **node,
        archi_app_plugin_instance_t *plugin_node,
        const char *vtable_symbol)
{
    if ((node == NULL) || (plugin_node == NULL) || (vtable_symbol == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    // Allocate a node
    archi_app_vtable_instance_t *vtable_node = malloc(sizeof(*vtable_node));
    if (vtable_node == NULL)
        return ARCHI_ERROR_ALLOC;

    *vtable_node = (archi_app_vtable_instance_t){.plugin_node = plugin_node};

    // Create plugin alias name string
    {
        size_t alias_len = strlen(plugin_node->base.name);
        size_t symbol_len = strlen(vtable_symbol);
        size_t size = alias_len + strlen(ARCHI_APP_VTABLE_ALIAS_SEPARATOR) + symbol_len + 1;

        char *alias_name = malloc(size);
        if (alias_name == NULL)
        {
            code = ARCHI_ERROR_ALLOC;
            goto failure;
        }

        snprintf(alias_name, size, "%s" ARCHI_APP_VTABLE_ALIAS_SEPARATOR "%s",
                plugin_node->base.name, vtable_symbol);

        vtable_node->base.name = alias_name;
    }

    // Get virtual table symbol
    vtable_node->vtable = archi_library_get_symbol(plugin_node->handle, vtable_symbol);
    if (vtable_node->vtable == NULL)
    {
        code = ARCHI_ERROR_SYMBOL;
        goto failure;
    }
    else if ((vtable_node->vtable->format.magic != ARCHI_API_MAGIC) ||
            (vtable_node->vtable->format.version != ARCHI_API_VERSION) ||
            (vtable_node->vtable->info.name == NULL))
    {
        code = ARCHI_ERROR_FORMAT;
        goto failure;
    }

    *node = vtable_node;
    return 0;

failure:
    archi_application_forget_vtable(vtable_node);
    return code;
}

void
archi_application_forget_vtable(
        archi_app_vtable_instance_t *node)
{
    if (node == NULL)
        return;

    archi_list_act_func_free_named((archi_list_node_t*)node, NULL);
}

archi_status_t
archi_application_initialize_context(
        archi_app_context_instance_t **node,
        archi_app_vtable_instance_t *vtable_node,
        const char *alias,
        const archi_list_t *config)
{
    if ((node == NULL) || (vtable_node == NULL) || (alias == NULL))
        return ARCHI_ERROR_MISUSE;

    if (vtable_node->vtable == NULL)
        return ARCHI_ERROR_MISUSE;
    else if (vtable_node->vtable->func.init_fn == NULL)
        return ARCHI_ERROR_FUNCTION;

    archi_status_t code;

    // Allocate a node
    archi_app_context_instance_t *context_node = malloc(sizeof(*context_node));
    if (context_node == NULL)
        return ARCHI_ERROR_ALLOC;

    *context_node = (archi_app_context_instance_t){.vtable_node = vtable_node};

    // Copy plugin alias name string
    {
        size_t alias_len = strlen(alias);

        char *alias_name = malloc(alias_len + 1);
        if (alias_name == NULL)
        {
            code = ARCHI_ERROR_ALLOC;
            goto failure;
        }

        memcpy(alias_name, alias, alias_len + 1);

        context_node->base.name = alias_name;
    }

    // Initialize context
    code = vtable_node->vtable->func.init_fn(&context_node->context, config);
    if (code != 0)
        goto failure;

    *node = context_node;
    return 0;

failure:
    archi_application_finalize_context(context_node);
    return code;
}

void
archi_application_finalize_context(
        archi_app_context_instance_t *node)
{
    if (node == NULL)
        return;

    if ((node->context != NULL) && (node->vtable_node != NULL) &&
            (node->vtable_node->vtable != NULL) &&
            (node->vtable_node->vtable->func.final_fn != NULL))
        node->vtable_node->vtable->func.final_fn(node->context);

    archi_list_act_func_free_named((archi_list_node_t*)node, NULL);
}

/*****************************************************************************/

static
archi_status_t
archi_application_load_plugin_impl(
        archi_list_node_t *node,
        void *data)
{
    if ((node == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_app_config_plugin_list_node_t *config_node = (archi_app_config_plugin_list_node_t*)node;
    struct archi_app_lists *lists = (struct archi_app_lists*)data;

    archi_status_t code;

    // Load a plugin
    archi_app_plugin_instance_t *plugin_node;
    code = archi_application_load_plugin(&plugin_node, config_node->base.name, config_node->pathname);
    if (code != 0)
        return code;

    // Append plugin node to the list
    code = archi_list_insert_node(lists->plugins, (archi_list_node_t*)plugin_node, NULL, NULL, false);
    if (code != 0)
        return code;

    for (size_t i = 0; i < config_node->num_vtables; i++)
    {
        // Get virtual table pointer
        archi_app_vtable_instance_t *vtable_node;
        code = archi_application_get_vtable(&vtable_node, plugin_node, config_node->vtable_symbols[i]);
        if (code != 0)
            return code;

        // Append vtable node to the list
        code = archi_list_insert_node(lists->vtables, (archi_list_node_t*)vtable_node, NULL, NULL, false);
        if (code != 0)
            return code;
    }

    return code;
}

static
archi_status_t
archi_application_unload_plugin_impl(
        archi_list_node_t *node,
        void *data)
{
    (void) data;

    archi_app_plugin_instance_t *plugin_node = (archi_app_plugin_instance_t*)node;
    if (plugin_node == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_application_unload_plugin(plugin_node);
    return 0;
}

static
archi_status_t
archi_application_forget_vtable_impl(
        archi_list_node_t *node,
        void *data)
{
    (void) data;

    archi_app_vtable_instance_t *vtable_node = (archi_app_vtable_instance_t*)node;
    if (vtable_node == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_application_forget_vtable(vtable_node);
    return 0;
}

archi_status_t
archi_application_load_plugins(
        archi_list_t *restrict plugins,
        archi_list_t *restrict vtables,

        const archi_list_t *restrict config,
        bool start_from_head)
{
    if ((plugins == NULL) || (vtables == NULL) || (config == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_list_t plugin_list = {0};
    archi_list_t vtable_list = {0};
    struct archi_app_lists lists = {.plugins = &plugin_list, .vtables = &vtable_list};

    archi_status_t code;

    // Load plugins and get virtual table pointers
    code = archi_list_traverse((archi_list_t*)config, NULL, NULL,
            archi_application_load_plugin_impl, &lists, start_from_head, 0, NULL);
    if (code < 0)
        goto failure;

    // Append plugin and virtual table instances to the lists
    code = archi_list_insert_sublist(plugins, &plugin_list, NULL, NULL, false);
    if (code != 0)
        goto failure;

    code = archi_list_insert_sublist(vtables, &vtable_list, NULL, NULL, false);
    if (code != 0)
        goto failure;

    return code;

failure:
    archi_list_remove_nodes(&vtable_list, NULL, NULL,
            archi_application_forget_vtable_impl, NULL, false, 0, NULL);

    archi_list_remove_nodes(&plugin_list, NULL, NULL,
            archi_application_unload_plugin_impl, NULL, false, 0, NULL);

    return code;
}

/*****************************************************************************/

static
archi_status_t
archi_application_finalize_context_impl(
        archi_list_node_t *node,
        void *data)
{
    (void) data;

    archi_app_context_instance_t *context_node = (archi_app_context_instance_t*)node;
    if (context_node == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_application_finalize_context(context_node);
    return 0;
}

static
archi_status_t
archi_application_perform_instruction(
        archi_list_node_t *node,
        void *data)
{
    if ((node == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_app_config_instruct_list_node_t *config_node = (archi_app_config_instruct_list_node_t*)node;
    struct archi_app_lists *lists = (struct archi_app_lists*)data;

    archi_status_t code;

    // Perform instructions
    switch (config_node->type)
    {
        case ARCHI_APP_CONFIG_INSTRUCT_INIT: // context initialization
            {
                archi_app_config_instruct_init_t instr = config_node->as_init;
                if ((instr.context_alias == NULL) || (instr.context_alias[0] == '\0'))
                    return ARCHI_ERROR_CONFIG;
                if (instr.vtable_alias == NULL)
                    return ARCHI_ERROR_CONFIG;

                // Search virtual table node starting from list tail
                archi_app_vtable_instance_t *vtable_node = NULL;
                code = archi_list_traverse(lists->vtables,
                        archi_list_node_func_select_by_name, instr.vtable_alias,
                        archi_list_act_func_copy_node, &vtable_node, false, 1, NULL);
                if (code < 0)
                    return code;
                else if (vtable_node == NULL)
                    return ARCHI_ERROR_CONFIG; // virtual table with such alias name was not found

                // Initialize context node
                archi_app_context_instance_t *context_node;
                code = archi_application_initialize_context(&context_node, vtable_node,
                        instr.context_alias, instr.config);
                if (code != 0)
                    return code;

                // Prepend/append context instance to the list
                code = archi_list_insert_node(lists->contexts,
                        (archi_list_node_t*)context_node, NULL, NULL, instr.prepend);
                if (code != 0)
                {
                    archi_application_finalize_context(context_node);
                    return code;
                }
            }
            break;

        case ARCHI_APP_CONFIG_INSTRUCT_FINAL: // context finalization
            {
                archi_app_config_instruct_final_t instr = config_node->as_final;
                if ((instr.context_alias == NULL) || (instr.context_alias[0] == '\0'))
                    return ARCHI_ERROR_CONFIG;

                // Remove context node
                size_t num_removed = 0;
                code = archi_list_remove_nodes(lists->contexts,
                        archi_list_node_func_select_by_name, instr.context_alias,
                        archi_application_finalize_context_impl, NULL, instr.start_from_head, 1, &num_removed);
                if (code < 0)
                    return code;
                else if (num_removed == 0)
                    return ARCHI_ERROR_CONFIG; // context with such alias name was not found
            }
            break;

        case ARCHI_APP_CONFIG_INSTRUCT_ASSIGN: // context assignment
            {
                archi_app_config_instruct_assign_t instr = config_node->as_assign;
                if ((instr.destination.context_alias == NULL) || (instr.source.context_alias == NULL))
                    return ARCHI_ERROR_CONFIG;
                else if (instr.destination.slot == NULL)
                    return ARCHI_ERROR_CONFIG;

                // Search destination context node starting from list tail
                archi_app_context_instance_t *dest_context_node = NULL;
                code = archi_list_traverse(lists->contexts,
                        archi_list_node_func_select_by_name, instr.destination.context_alias,
                        archi_list_act_func_copy_node, &dest_context_node, false, 1, NULL);
                if (code < 0)
                    return code;
                else if (dest_context_node == NULL)
                    return ARCHI_ERROR_CONFIG; // context with such alias name was not found
                else if (dest_context_node->vtable_node == NULL)
                    return ARCHI_ERROR_MISUSE;
                else if (dest_context_node->vtable_node->vtable == NULL)
                    return ARCHI_ERROR_MISUSE;
                else if (dest_context_node->vtable_node->vtable->func.set_fn == NULL)
                    return ARCHI_ERROR_FUNCTION;

                // Search source context node starting from list tail
                archi_app_context_instance_t *src_context_node = NULL;
                code = archi_list_traverse(lists->contexts,
                        archi_list_node_func_select_by_name, instr.source.context_alias,
                        archi_list_act_func_copy_node, &src_context_node, false, 1, NULL);
                if (code < 0)
                    return code;
                else if (src_context_node == NULL)
                    return ARCHI_ERROR_CONFIG; // context with such alias name was not found
                else if (instr.source.slot != NULL)
                {
                    if (src_context_node->vtable_node == NULL)
                        return ARCHI_ERROR_MISUSE;
                    else if (src_context_node->vtable_node->vtable == NULL)
                        return ARCHI_ERROR_MISUSE;
                    else if (src_context_node->vtable_node->vtable->func.get_fn == NULL)
                        return ARCHI_ERROR_FUNCTION;
                }

                // Perform the assignment
                archi_value_t value = {0};

                if (instr.source.slot != NULL) // get value from the source context slot
                {
                    code = src_context_node->vtable_node->vtable->func.get_fn(
                            src_context_node->context, instr.source.slot, &value);
                    if (code != 0)
                        return code;
                }
                else // value is the whole source context
                {
                    value.ptr = src_context_node->context;
                    value.num_of = 1;
                    value.type = ARCHI_VALUE_DATA;
                }

                code = dest_context_node->vtable_node->vtable->func.set_fn(
                        dest_context_node->context, instr.destination.slot, &value);
                if (code != 0)
                    return code;
            }
            break;

        case ARCHI_APP_CONFIG_INSTRUCT_ACT: // context action
            {
                archi_app_config_instruct_act_t instr = config_node->as_act;
                if (instr.context_alias == NULL)
                    return ARCHI_ERROR_CONFIG;

                // Search context node starting from list tail
                archi_app_context_instance_t *context_node = NULL;
                code = archi_list_traverse(lists->contexts,
                        archi_list_node_func_select_by_name, instr.context_alias,
                        archi_list_act_func_copy_node, &context_node, false, 1, NULL);
                if (code < 0)
                    return code;
                else if (context_node == NULL)
                    return ARCHI_ERROR_CONFIG; // context with such alias name was not found
                else if (context_node->vtable_node == NULL)
                    return ARCHI_ERROR_MISUSE;
                else if (context_node->vtable_node->vtable == NULL)
                    return ARCHI_ERROR_MISUSE;
                else if (context_node->vtable_node->vtable->func.act_fn == NULL)
                    return ARCHI_ERROR_FUNCTION;

                // Perform the action
                code = context_node->vtable_node->vtable->func.act_fn(
                        context_node->context, instr.action_type, instr.config);
                if (code != 0)
                    return code;
            }
            break;

        default: // unknown instruction type
            return ARCHI_ERROR_CONFIG;
    }

    return code;
}

archi_status_t
archi_application_initialize_contexts(
        archi_list_t *restrict contexts,

        const archi_list_t *restrict vtables,

        const archi_list_t *restrict config,
        bool start_from_head)
{
    if ((contexts == NULL) || (vtables == NULL) || (config == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_list_t context_list = {0};
    struct archi_app_lists lists = {.vtables = (archi_list_t*)vtables, .contexts = &context_list};

    archi_status_t code;

    // Initialize contexts and perform initialization instructions
    code = archi_list_traverse((archi_list_t*)config, NULL, NULL,
            archi_application_perform_instruction, &lists, start_from_head, 0, NULL);
    if (code < 0)
        goto failure;

    // Append context instances to the list
    code = archi_list_insert_sublist(contexts, &context_list, NULL, NULL, false);
    if (code != 0)
        goto failure;

    return code;

failure:
    archi_list_remove_nodes(&context_list, NULL, NULL,
            archi_application_finalize_context_impl, NULL, false, 0, NULL);

    return code;
}

/*****************************************************************************/

archi_status_t
archi_application_initialize(
        archi_application_t *app,
        const archi_app_configuration_t *config)
{
    if (app == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_status_t code;

    code = archi_application_load_plugins(&app->plugins, &app->vtables, &config->plugins, true);
    if (code != 0)
        goto failure;

    code = archi_application_initialize_contexts(&app->contexts, &app->vtables, &config->instructions, true);
    if (code != 0)
        goto failure;

    return code;

failure:
    archi_application_finalize(app);
    return code;
}

void
archi_application_finalize(
        archi_application_t *app)
{
    if (app == NULL)
        return;

    archi_list_remove_nodes(&app->contexts, NULL, NULL,
            archi_application_finalize_context_impl, NULL, false, 0, NULL);

    archi_list_remove_nodes(&app->vtables, NULL, NULL,
            archi_application_forget_vtable_impl, NULL, false, 0, NULL);

    archi_list_remove_nodes(&app->plugins, NULL, NULL,
            archi_application_unload_plugin_impl, NULL, false, 0, NULL);

    *app = (archi_application_t){0};
}

