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
 * @brief Application instance types.
 */

#pragma once
#ifndef _ARCHI_APP_INSTANCE_TYP_H_
#define _ARCHI_APP_INSTANCE_TYP_H_

#include "archi/util/list.typ.h"

struct archi_plugin_vtable;

/**
 * @brief Plugin instance.
 */
typedef struct archi_app_plugin_instance {
    archi_list_node_named_t base; ///< Named node base.

    void *handle; ///< Plugin library handle.
} archi_app_plugin_instance_t;

/**
 * @brief Virtual table instance.
 */
typedef struct archi_app_vtable_instance {
    archi_list_node_named_t base; ///< Named node base.

    const struct archi_plugin_vtable *vtable; ///< Pointer to virtual table.
    archi_app_plugin_instance_t *plugin_node; ///< Parent plugin instance.
} archi_app_vtable_instance_t;

/**
 * @brief Context instance.
 */
typedef struct archi_app_context_instance {
    archi_list_node_named_t base; ///< Named node base.

    void *context; ///< Pointer to context.
    archi_app_vtable_instance_t *vtable_node; ///< Parent vtable instance.
} archi_app_context_instance_t;

/**
 * @brief Application instance.
 */
typedef struct archi_application {
    archi_list_t plugins;  ///< List of plugins.
    archi_list_t vtables;  ///< List of virtual tables.
    archi_list_t contexts; ///< List of contexts.
} archi_application_t;

#endif // _ARCHI_APP_INSTANCE_TYP_H_

