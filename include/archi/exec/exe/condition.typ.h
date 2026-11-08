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
 * @brief Data for transition functions for branching in directed execution graphs.
 */

#pragma once
#ifndef _ARCHI_EXEC_EXE_CONDITION_TYP_H_
#define _ARCHI_EXEC_EXE_CONDITION_TYP_H_

#include "archi_base/pointer.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


/**
 * @brief Signature of a condition function.
 *
 * @return The successor node, or NULL.
 */
#define ARCHI_DEXGRAPH_CONDITION_FUNC(func_name)    bool func_name( \
        archi_pointer_t data, /* [in] Condition function data. */   \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Condition function type.
 */
typedef ARCHI_DEXGRAPH_CONDITION_FUNC((*archi_dexgraph_condition_func_t));

#endif // _ARCHI_EXEC_EXE_CONDITION_TYP_H_

