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
 * @brief Application configuration functions.
 */

#pragma once
#ifndef _ARCHI_APP_CONFIG_FUN_H_
#define _ARCHI_APP_CONFIG_FUN_H_

#include "archi/app/config.typ.h"
#include "archi/util/status.typ.h"

struct archi_application;
struct archi_context_interface;
struct archi_list_node_named_value;

/**
 * @brief Initialize a context and add it to an application.
 *
 * @return Status code.
 */
archi_status_t
archi_app_add_context(
        struct archi_application *app, ///< [in] Application instance.
        const void *key, ///< [in] Context key.
        const struct archi_context_interface *interface, ///< [in] Context interface.
        const struct archi_list_node_named_value *config ///< [in] Context configuration.
);

/**
 * @brief Finalize a context and remove it from an application.
 *
 * @return Status code.
 */
archi_status_t
archi_app_remove_context(
        struct archi_application *app, ///< [in] Application instance.
        const void *key ///< [in] Context key.
);

/*****************************************************************************/

/**
 * @brief Do a configuration step for an application.
 *
 * @return Status code.
 */
archi_status_t
archi_app_do_config_step(
        struct archi_application *app, ///< [in] Application instance.
        archi_app_config_step_t step ///< [in] Configuration step.
);

/**
 * @brief Undo a configuration step for an application.
 *
 * If the step is context initialization, the corresponding context
 * is finalized and removed from the application.
 * Other step types are ignored.
 *
 * @return Status code.
 */
archi_status_t
archi_app_undo_config_step(
        struct archi_application *app, ///< [in] Application instance.
        archi_app_config_step_t step ///< [in] Configuration step.
);

#endif // _ARCHI_APP_CONFIG_FUN_H_

