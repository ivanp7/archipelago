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
 * @brief Application interface types.
 */

#pragma once
#ifndef _ARCHI_APP_INSTANCE_TYP_H_
#define _ARCHI_APP_INSTANCE_TYP_H_

#include "archi/util/container.typ.h"

#include <stddef.h> // for size_t

struct archi_app_loader_library;
struct archi_app_loader_library_symbol;
struct archi_app_config_step;

/**
 * @brief Application configuration.
 */
typedef struct archi_app_config {
    struct archi_app_loader_library *libraries;         ///< Libraries to load.
    struct archi_app_loader_library_symbol *interfaces; ///< Interfaces to get.
    struct archi_app_config_step *steps;                ///< Configuration steps to do.

    size_t num_libraries;  ///< Number of libraries to load.
    size_t num_interfaces; ///< Number of interfaces to get.
    size_t num_steps;      ///< Number of configuration steps to do.
} archi_app_config_t;

/**
 * @brief Application instance.
 */
typedef struct archi_application {
    archi_container_t libraries;  ///< Loaded libraries (element type is library handle pointer).
    archi_container_t interfaces; ///< Available context interfaces (element type: archi_context_interface_t).
    archi_container_t contexts;   ///< Initialized contexts (element type: archi_context_t).
} archi_application_t;

#endif // _ARCHI_APP_INSTANCE_TYP_H_

