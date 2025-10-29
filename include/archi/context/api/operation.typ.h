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
 * @brief Types for context operations.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_API_OPERATION_TYP_H_
#define _ARCHI_CONTEXT_API_OPERATION_TYP_H_

#include "archi/context/api/handle.typ.h"
#include "archipelago/base/pointer.typ.h"
#include "archipelago/base/error.typ.h"

/**
 * @brief Declare/define a context operation function.
 *
 * This function implements a context operation.
 */
#define ARCHI_CONTEXT_OPERATION_FUNC(name)   void name(                     \
        archi_context_t context, /* [in] Context. */                        \
        archi_rcpointer_t argument, /* [in] Context operation argument. */  \
        ARCHI_ERROR_PARAMETER_DECL) /* [out] Error. */

/**
 * @brief Context operation function.
 */
typedef ARCHI_CONTEXT_OPERATION_FUNC((*archi_context_operation_func_t));

/**
 * @brief Function type tag for context operation functions.
 */
#define ARCHI_POINTER_FUNCTION_TAG__CONTEXT_OPERATION   1

#endif // _ARCHI_CONTEXT_API_OPERATION_TYP_H_

