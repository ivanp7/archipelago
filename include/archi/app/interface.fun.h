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
 * @brief Application interface.
 */

#pragma once
#ifndef _ARCHI_APP_INTERFACE_FUN_H_
#define _ARCHI_APP_INTERFACE_FUN_H_

#include "archi/util/status.typ.h"

struct archi_app_plugin_instance;
struct archi_app_vtable_instance;
struct archi_app_context_instance;

struct archi_list_node;
struct archi_list;

struct archi_application;
struct archi_app_configuration;

/**
 * @brief Allocate an instance and load a plugin.
 *
 * @return Zero on success, non-zero value on error.
 */
archi_status_t
archi_application_load_plugin(
        struct archi_app_plugin_instance **node, ///< [out] Place for a loaded plugin instance.
        const char *alias, ///< [in] Plugin alias name.
        const char *pathname ///< [in] Pathname of the plugin file.
);

/**
 * @brief Unload a plugin and free instance memory.
 */
void
archi_application_unload_plugin(
        struct archi_app_plugin_instance *node ///< [in] Loaded plugin instance.
);

/**
 * @brief Allocate an instance and get a virtual table.
 *
 * @return Zero on success, non-zero value on error.
 */
archi_status_t
archi_application_get_vtable(
        struct archi_app_vtable_instance **node, ///< [out] Place for a virtual table instance.
        struct archi_app_plugin_instance *plugin_node, ///< [in] Loaded plugin instance.
        const char *vtable_symbol ///< [in] Virtual table symbol name.
);

/**
 * @brief Free instance memory.
 */
void
archi_application_forget_vtable(
        struct archi_app_vtable_instance *node ///< [in] Virtual table instance.
);

/**
 * @brief Allocate an instance and initialize a context.
 *
 * @return Zero on success, non-zero value on error.
 */
archi_status_t
archi_application_initialize_context(
        struct archi_app_context_instance **node, ///< [out] Place for a context instance.
        struct archi_app_vtable_instance *vtable_node, ///< [in] Virtual table instance.
        const char *alias, ///< [in] Context alias name.
        const struct archi_list *config ///< [in] Context configuration.
);

/**
 * @brief Finalize a context and free instance memory.
 */
void
archi_application_finalize_context(
        struct archi_app_context_instance *node ///< [in] Context instance.
);

/*****************************************************************************/

/**
 * @brief Load plugins, get virtual tables, and append them to output lists.
 *
 * @return Status code.
 */
archi_status_t
archi_application_load_plugins(
        struct archi_list *restrict plugins, ///< [in,out] List of loaded plugins.
        struct archi_list *restrict vtables, ///< [in,out] List of virtual tables.

        const struct archi_list *restrict config, ///< [in] Configuration of plugins.
        bool start_from_head ///< [in] Whether to start traversing from the head.
);

/**
 * @brief Initialize contexts and perform initialization instructions.
 *
 * @return Status code.
 */
archi_status_t
archi_application_initialize_contexts(
        struct archi_list *restrict contexts, ///< [in,out] List of initialized contexts.

        const struct archi_list *restrict vtables, ///< [in] List of virtual tables.

        const struct archi_list *restrict config, ///< [in] Initialization instructions.
        bool start_from_head ///< [in] Whether to start traversing from the head.
);

/*****************************************************************************/

/**
 * @brief Initialize the application from configuration.
 *
 * @return Status code.
 */
archi_status_t
archi_application_initialize(
        struct archi_application *app, ///< [in,out] Application object.
        const struct archi_app_configuration *config ///< [in] Application configuration.
);

/**
 * @brief Finalize the application.
 *
 * This function does not free the structure pointed to by 'app' pointer itself.
 * If it is allocated on heap, it must be freed manually.
 *
 * This function is idempotent.
 */
void
archi_application_finalize(
        struct archi_application *app ///< [in] Application object.
);

#endif // _ARCHI_APP_INTERFACE_FUN_H_

