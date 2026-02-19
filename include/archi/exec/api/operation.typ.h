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
 * @brief Types for directed execution graph operations.
 */

#pragma once
#ifndef _ARCHI_EXEC_API_OPERATION_TYP_H_
#define _ARCHI_EXEC_API_OPERATION_TYP_H_

#include "archi_base/error.typ.h"


/**
 * @brief Signature of an operation function.
 */
#define ARCHI_DEXGRAPH_OPERATION_FUNC(func_name)    void func_name( \
        void *data, /* [in] Operation function data. */             \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Operation function type.
 */
typedef ARCHI_DEXGRAPH_OPERATION_FUNC((*archi_dexgraph_operation_func_t));

/**
 * @brief Directed execution graph operation.
 */
typedef struct archi_dexgraph_operation {
    archi_dexgraph_operation_func_t function; ///< Operation function.
    void *data; ///< Operation function data.
} archi_dexgraph_operation_t;

#endif // _ARCHI_EXEC_API_OPERATION_TYP_H_

