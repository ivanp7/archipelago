/*****************************************************************************
 * Copyright (C) 2023-2026 by Ivan Podmazov                                  *
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
 * @brief Data for context registry operations.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_API_REGISTRY_TYP_H_
#define _ARCHI_CONTEXT_API_REGISTRY_TYP_H_

#include "archi_base/kvlist.typ.h"


/**
 * @brief Description of a parameter list for a context interface function.
 */
typedef struct archi_context_registry_params {
    const char *context_key; ///< Key of parameter list context (dynamic parameter list).
    archi_krcvlist_t *list; ///< Static parameter list.
} archi_context_registry_params_t;

#endif // _ARCHI_CONTEXT_API_REGISTRY_TYP_H_

