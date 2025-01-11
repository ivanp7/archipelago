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
 * @brief Application configuration functions.
 */

#pragma once
#ifndef _ARCHI_APP_CONFIG_FUN_H_
#define _ARCHI_APP_CONFIG_FUN_H_

#include "archi/util/container.typ.h"

/**
 * @brief Container element function for configuration step.
 *
 * Container element type is expected to be archi_app_config_step_t.
 */
ARCHI_CONTAINER_ELEMENT_FUNC(archi_app_apply_config_step);

/**
 * @brief Data for configuration step function.
 */
typedef struct archi_app_apply_config_step_data {
    archi_container_t contexts; ///< Contexts of application.
    archi_container_t context_interfaces; ///< Available context interfaces.
} archi_app_apply_config_step_data_t;

#endif // _ARCHI_APP_CONFIG_FUN_H_

