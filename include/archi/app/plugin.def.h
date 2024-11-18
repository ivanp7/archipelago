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
 * @brief Constants and macros for application plugins.
 */

#pragma once
#ifndef _ARCHI_APP_PLUGIN_DEF_H_
#define _ARCHI_APP_PLUGIN_DEF_H_

#include "archi/util/list.typ.h"
#include "archi/util/status.typ.h"

/**
 * @brief Declare/define plugin help function.
 *
 * This function is intended to provide documentation.
 *
 * @see archi_plugin_help_func_t
 *
 * @return Status code.
 */
#define ARCHI_PLUGIN_HELP_FUNC(func_name) archi_status_t func_name( \
        const char *topic) /* Help topic. */

/**
 * @brief Declare/define plugin context initialization function.
 *
 * This function is intended to initialize/allocate resources (contexts).
 *
 * @see archi_plugin_init_func_t
 *
 * @return Status code.
 */
#define ARCHI_PLUGIN_INIT_FUNC(func_name) archi_status_t func_name( \
        void **const restrict context, /* Place for pointer to the new context. */ \
        const archi_list_t *const restrict config) /* Context configuration. */

/**
 * @brief Declare/define plugin context finalization function.
 *
 * This function is intended to finalize/release resources (contexts).
 *
 * @see archi_plugin_final_func_t
 */
#define ARCHI_PLUGIN_FINAL_FUNC(func_name) void func_name( \
        void *const context) /* Context to destroy. */

/**
 * @brief Declare/define plugin action inputs setter function.
 *
 * This function is intended to store input data in a context.
 *
 * @see archi_plugin_set_func_t
 *
 * @return Status code.
 */
#define ARCHI_PLUGIN_SET_FUNC(func_name) archi_status_t func_name( \
        void *const restrict context, /* Action context. */ \
        const char *const restrict port, /* Action input port. */ \
        const archi_value_t *const restrict value) /* Node with input value. */

/**
 * @brief Declare/define plugin action outputs getter function.
 *
 * This function is intended to retrieve output data from a context.
 *
 * @see archi_plugin_get_func_t
 *
 * @return Status code.
 */
#define ARCHI_PLUGIN_GET_FUNC(func_name) archi_status_t func_name( \
        void *const restrict context, /* Action context. */ \
        const char *const restrict port, /* Action output port. */ \
        archi_value_t *const restrict value) /* Node for output value. */

/**
 * @brief Declare/define plugin action function.
 *
 * This function is intended to perform actions within contexts.
 *
 * @see archi_plugin_act_func_t
 *
 * @return Status code.
 */
#define ARCHI_PLUGIN_ACT_FUNC(func_name) archi_status_t func_name( \
        void *const restrict context, /* Action context. */ \
        const char *const restrict action, /* Action type. */ \
        const archi_list_t *const restrict config) /* Action configuration. */

/*****************************************************************************/

/**
 * @brief Default plugin virtual table symbol.
 */
#define ARCHI_PLUGIN_VTABLE_DEFAULT plugin_vtable

#endif // _ARCHI_APP_PLUGIN_DEF_H_

