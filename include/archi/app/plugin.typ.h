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
 * @brief Types for application plugins.
 */

#pragma once
#ifndef _ARCHI_APP_PLUGIN_TYP_H_
#define _ARCHI_APP_PLUGIN_TYP_H_

#include "archi/app/plugin.def.h"

#include <stdint.h> // for uint32_t

typedef ARCHI_PLUGIN_HELP_FUNC((*archi_plugin_help_func_t));    ///< Plugin help function.
typedef ARCHI_PLUGIN_INIT_FUNC((*archi_plugin_init_func_t));    ///< Context initialization function.
typedef ARCHI_PLUGIN_FINAL_FUNC((*archi_plugin_final_func_t));  ///< Context finalization function.
typedef ARCHI_PLUGIN_SET_FUNC((*archi_plugin_set_func_t));      ///< Action inputs setter function.
typedef ARCHI_PLUGIN_GET_FUNC((*archi_plugin_get_func_t));      ///< Action outputs getter function.
typedef ARCHI_PLUGIN_ACT_FUNC((*archi_plugin_act_func_t));      ///< Action function.

/**
 * @brief Plugin virtual table.
 */
typedef struct archi_plugin_vtable {
    struct {
        uint32_t magic;   ///< Value uniquely identifying plugin format, should be equal to ARCHI_API_MAGIC.
        uint32_t version; ///< Value determining application-plugin compatibility, should be equal to ARCHI_API_VERSION.
    } format; ///< Plugin format header.

    struct {
        char *name; ///< Plugin virtual table name.

        char *description; ///< Plugin virtual table description.
        archi_plugin_help_func_t help_fn; ///< Help function.
    } info; ///< Plugin meta information and documentation.

    struct {
        archi_plugin_init_func_t init_fn;   ///< Context initialization function.
        archi_plugin_final_func_t final_fn; ///< Context finalization function.

        archi_plugin_set_func_t set_fn; ///< Action inputs setter function.
        archi_plugin_get_func_t get_fn; ///< Action outputs getter function.

        archi_plugin_act_func_t act_fn; ///< Action function.
    } func; ///< Plugin domain logic.
} archi_plugin_vtable_t;

#endif // _ARCHI_APP_PLUGIN_TYP_H_

