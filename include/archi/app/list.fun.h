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
 * @brief Context interface for named value lists used in context interfaces.
 */

#pragma once
#ifndef _ARCHI_APP_LIST_FUN_H_
#define _ARCHI_APP_LIST_FUN_H_

#include "archi/app/context.typ.h"

ARCHI_CONTEXT_INIT_FUNC(archi_app_value_list_init);   ///< Value list initialization function.
ARCHI_CONTEXT_FINAL_FUNC(archi_app_value_list_final); ///< Value list finalization function.
ARCHI_CONTEXT_SET_FUNC(archi_app_value_list_set);     ///< Value list setter function.
ARCHI_CONTEXT_GET_FUNC(archi_app_value_list_get);     ///< Value list getter function.

extern
const archi_context_interface_t archi_app_value_list_interface; ///< Value list interface functions.

/**
 * @brief Key of value list interface.
 */
#define ARCHI_APP_VALUE_LIST_INTERFACE "values"

#endif // _ARCHI_APP_LIST_FUN_H_

