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
 * @brief Application context interface.
 */

#pragma once
#ifndef _ARCHI_EXE_CONTEXT_FUN_H_
#define _ARCHI_EXE_CONTEXT_FUN_H_

#include "archi/exe/context.typ.h"
#include "archi/app/context.typ.h"

ARCHI_CONTEXT_SET_FUNC(archi_app_context_set); ///< Context slot setter function.
ARCHI_CONTEXT_GET_FUNC(archi_app_context_get); ///< Context slot getter function.

extern
const archi_context_interface_t archi_app_context_interface; ///< Context interface functions.

/*****************************************************************************/

enum {
    ARCHI_SHM_ADDR = 0, ///< Shared memory address (always stored in the first header pointer).

    ARCHI_SHM_PTR_SIGNAL_WATCH_SET,  ///< Pointer to signal watch set.
    ARCHI_SHM_PTR_PLUGIN_LIBRARIES,  ///< Pointer to list of plugin libraries to load.
    ARCHI_SHM_PTR_PLUGIN_INTERFACES, ///< Pointer to list of plugin interfaces to get.
    ARCHI_SHM_PTR_APP_CONFIG_STEPS,  ///< Pointer to application configuration steps container.

    ARCHI_SHM_NUM_HEADER_POINTERS, ///< Number of pointers in a shared memory header.
};

/**
 * @brief Parse shared memory and extract application configuration.
 */
void
archi_app_parse_shm(
        void **restrict shmaddr, ///< [in] Shared memory header.

        const archi_signal_watch_set_t *restrict *restrict signal_watch_set, ///< [out] Signal watch set.
        archi_container_t *restrict plugin_libraries,  ///< [out] Plugin libraries to load.
        archi_container_t *restrict plugin_interfaces, ///< [out] Plugin interfaces to get.
        archi_container_t *restrict app_config_steps   ///< [out] Application configuration steps.
);

/**
 * @brief Initialize an application object.
 *
 * @return Status code.
 */
archi_status_t
archi_app_initialize(
        archi_application_t *app, ///< [out] Application to initialize.

        const archi_signal_watch_set_t *signal_watch_set ///< [in] Signal watch set.
);

/**
 * @brief Configure an application.
 *
 * @return Status code.
 */
archi_status_t
archi_app_configure(
        archi_application_t *app, ///< [in,out] Application to configure.

        archi_container_t plugin_libraries,  ///< [in] Plugin libraries to load.
        archi_container_t plugin_interfaces, ///< [in] Plugin interfaces to get.
        archi_container_t app_config_steps   ///< [in] Application configuration steps.
);

/**
 * @brief Finalize an application.
 */
void
archi_app_finalize(
        archi_application_t *app ///< [in] Application to finalialize.
);

#endif // _ARCHI_EXE_CONTEXT_FUN_H_

