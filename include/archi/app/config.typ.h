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
 * @brief Application configuration types.
 */

#pragma once
#ifndef _ARCHI_APP_CONFIG_TYP_H_
#define _ARCHI_APP_CONFIG_TYP_H_

#include "archi/util/list.typ.h"

/**
 * @brief Configuration node for plugins and virtual tables.
 */
typedef struct archi_app_config_plugin_list_node {
    archi_list_node_named_t base; ///< Node base.

    char *pathname; ///< Pathname of the plugin file.

    size_t num_vtables; ///< Number of virtual tables.
    char **vtable_symbols; ///< Virtual table symbol names.
} archi_app_config_plugin_list_node_t;

/*****************************************************************************/

/**
 * @brief Type of initialization instructions.
 */
typedef enum archi_app_config_instruct_type {
    ARCHI_APP_CONFIG_INSTRUCT_INIT,   ///< Initialize a context.
    ARCHI_APP_CONFIG_INSTRUCT_FINAL,  ///< Finalize a context.
    ARCHI_APP_CONFIG_INSTRUCT_ASSIGN, ///< Assign a value to context (get and set).
    ARCHI_APP_CONFIG_INSTRUCT_ACT,    ///< Perform a context action.
} archi_app_config_instruct_type_t;

/**
 * @brief Context initialization instruction.
 *
 * Empty alias name is forbidden.
 */
typedef struct archi_app_config_instruct_init {
    char *context_alias; ///< Context alias name.
    bool prepend; ///< Whether to prepend the new context node to the list, or to append it.

    char *vtable_alias; ///< Virtual table alias name.
    archi_list_t *config; ///< Context configuration.
} archi_app_config_instruct_init_t;

/**
 * @brief Context finalization instruction.
 *
 * Empty alias name is forbidden.
 */
typedef struct archi_app_config_instruct_final {
    char *context_alias; ///< Context alias name.
    bool start_from_head; ///< Whether to start from list head searching for the context node.
} archi_app_config_instruct_final_t;

/**
 * @brief Context assignment instruction.
 *
 * Context alias names must not be null.
 * Destination slot must not be null.
 *
 * Source slot may be null. In that case, source context pointer
 * is passed to destination setter function.
 */
typedef struct archi_app_config_instruct_assign {
    struct {
        char *context_alias; ///< Context alias name.
        char *slot; ///< Context slot.
    } destination,
        source;
} archi_app_config_instruct_assign_t;

/**
 * @brief Context action instruction.
 */
typedef struct archi_app_config_instruct_act {
    char *context_alias; ///< Context alias name.

    char *action_type; ///< Action type.
    archi_list_t *config; ///< Action configuration.
} archi_app_config_instruct_act_t;

/**
 * @brief Configuration node for initialization instructions.
 */
typedef struct archi_app_config_instruct_list_node {
    archi_list_node_t base; ///< Node base.

    archi_app_config_instruct_type_t type; ///< Instruction type.
    union {
        archi_app_config_instruct_init_t as_init;     ///< Context initialization.
        archi_app_config_instruct_final_t as_final;   ///< Context finalization.
        archi_app_config_instruct_assign_t as_assign; ///< Context assignment.
        archi_app_config_instruct_act_t as_act;       ///< Context action.
    };
} archi_app_config_instruct_list_node_t;

/*****************************************************************************/

/**
 * @brief Application configuration.
 */
typedef struct archi_app_configuration {
    archi_list_t plugins;       ///< List of plugins.
    archi_list_t instructions;  ///< List of initialization instructions.
} archi_app_configuration_t;

#endif // _ARCHI_APP_CONFIG_TYP_H_

