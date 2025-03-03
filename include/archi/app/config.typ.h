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
 * @brief Types for application configuration.
 */

#pragma once
#ifndef _ARCHI_APP_CONFIG_TYP_H_
#define _ARCHI_APP_CONFIG_TYP_H_

#include <stdbool.h>

struct archi_list_node_named_value;
struct archi_value;

/**
 * @brief Specific configuration step data for context initialization.
 *
 * @warning Null key is forbidden.
 */
typedef struct archi_app_config_step_init {
    const char *interface_key; ///< Context interface key.

    bool dynamic_config; ///< Whether is context configuration is provided as a key of object.
    union {
        const struct archi_list_node_named_value *node; ///< Head node of context configuration.
        const char *key; ///< Key of context configuration object.
    } config;
} archi_app_config_step_init_t;

/**
 * @brief Specific configuration step data for context slot value setting.
 */
typedef struct archi_app_config_step_set {
    const char *slot; ///< Context slot.
    const struct archi_value *value; ///< Value to set.
} archi_app_config_step_set_t;

/**
 * @brief Specific configuration step data for context assignment.
 *
 * Destination slot must not be null.
 * Source slot may be null. In that case,
 * source context pointer is passed to destination setter function.
 */
typedef struct archi_app_config_step_assign {
    const char *slot; ///< Destination context slot.
    const char *source_key; ///< Source context key.
    const char *source_slot; ///< Source context slot.
} archi_app_config_step_assign_t;

/**
 * @brief Specific configuration step data for context action.
 */
typedef struct archi_app_config_step_act {
    const char *action; ///< Action type.

    bool dynamic_params; ///< Whether are action parameters are provided as a key of object.
    union {
        const struct archi_list_node_named_value *node; ///< Head node of action parameters.
        const char *key; ///< Key of action parameters object.
    } params;
} archi_app_config_step_act_t;

/*****************************************************************************/

/**
 * @brief Type of application configuration steps.
 */
typedef enum archi_app_config_step_type {
    ARCHI_APP_CONFIG_STEP_INIT,     ///< Initialize a context.
    ARCHI_APP_CONFIG_STEP_FINAL,    ///< Finalize a context.
    ARCHI_APP_CONFIG_STEP_SET,      ///< Set a value to context slot.
    ARCHI_APP_CONFIG_STEP_ASSIGN,   ///< Assign a value to context slot (get -> set).
    ARCHI_APP_CONFIG_STEP_ACT,      ///< Perform a context action.
} archi_app_config_step_type_t;

/**
 * @brief Application configuration step.
 */
typedef struct archi_app_config_step {
    archi_app_config_step_type_t type; ///< Step type.

    const char *key; ///< Key of the context being acted on.

    union {
        archi_app_config_step_init_t as_init;     ///< Context initialization.
        // no 'as_final' here
        archi_app_config_step_set_t as_set;       ///< Context slot value setting.
        archi_app_config_step_assign_t as_assign; ///< Context assignment.
        archi_app_config_step_act_t as_act;       ///< Context action.
    };
} archi_app_config_step_t;

#endif // _ARCHI_APP_CONFIG_TYP_H_

