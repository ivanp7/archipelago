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

struct archi_value;

/**
 * @brief Configuration step for context initialization.
 *
 * @warning Null key is forbidden.
 */
typedef struct archi_app_config_step_init {
    const void *key; ///< Context key.

    const void *interface_key; ///< Context interface key.
    const void *config; ///< Context configuration.
} archi_app_config_step_init_t;

/**
 * @brief Configuration step for context finalization.
 */
typedef struct archi_app_config_step_final {
    const void *key; ///< Context key.
} archi_app_config_step_final_t;

/**
 * @brief Configuration step for context slot value setting.
 */
typedef struct archi_app_config_step_set {
    const void *key; ///< Context key.

    const char *slot; ///< Context slot.
    const struct archi_value *value; ///< Value to set.
} archi_app_config_step_set_t;

/**
 * @brief Configuration step for context assignment.
 *
 * Destination slot must not be null.
 * Source slot may be null. In that case,
 * source context pointer is passed to destination setter function.
 */
typedef struct archi_app_config_step_assign {
    struct {
        const void *key; ///< Context key.
        const char *slot; ///< Context slot.
    } destination, ///< Assignment destination.
        source; ///< Assignment source.
} archi_app_config_step_assign_t;

/**
 * @brief Configuration step for context action.
 */
typedef struct archi_app_config_step_act {
    const void *key; ///< Context key.

    const char *action; ///< Action type.
    const void *params; ///< Action parameters.
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

    union {
        archi_app_config_step_init_t as_init;     ///< Context initialization.
        archi_app_config_step_final_t as_final;   ///< Context finalization.
        archi_app_config_step_set_t as_set;       ///< Context slot value setting.
        archi_app_config_step_assign_t as_assign; ///< Context assignment.
        archi_app_config_step_act_t as_act;       ///< Context action.
    };
} archi_app_config_step_t;

#endif // _ARCHI_APP_CONFIG_TYP_H_

