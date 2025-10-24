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
 * @brief Context interface for parameter lists.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_CTX_PARAMETERS_VAR_H_
#define _ARCHI_CONTEXT_CTX_PARAMETERS_VAR_H_

#include "archi/context/api/interface.typ.h"

/**
 * @brief Parameter list initialization function.
 *
 * Accepts any parameters, all of them are added to the list.
 */
ARCHI_CONTEXT_INIT_FUNC(archi_context_parameters_init);

/**
 * @brief Parameter list finalization function.
 */
ARCHI_CONTEXT_FINAL_FUNC(archi_context_parameters_final);

/**
 * @brief Parameter list getter function.
 *
 * Provides any slot name with no indices, returns a parameter with such name.
 */
ARCHI_CONTEXT_GET_FUNC(archi_context_parameters_get);

/**
 * @brief Parameter list setter function.
 *
 * Accepts any slot name with no indices, adds a parameter with such name on top of the list.
 */
ARCHI_CONTEXT_SET_FUNC(archi_context_parameters_set);

/**
 * @brief Parameter list action function.
 *
 * Provides the following actions:
 * - "_" : prepend the parameter list with the action parameters
 */
ARCHI_CONTEXT_ACT_FUNC(archi_context_parameters_act);

/**
 * @brief Parameter list interface.
 */
extern
const archi_context_interface_t archi_context_parameters_interface;

#endif // _ARCHI_CONTEXT_CTX_PARAMETERS_VAR_H_

