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
 * @brief Context callback type.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_API_CALLBACK_TYP_H_
#define _ARCHI_CONTEXT_API_CALLBACK_TYP_H_

#include "archi_base/pointer.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Declare/define context callback function.
 *
 * This function is intended for processing resources obtained from a context.
 *
 * @note Callback functions should be called exactly once, or, in case of error, never.
 */
#define ARCHI_CONTEXT_CALLBACK_FUNC(func_name)  void func_name(             \
        archi_rcpointer_t value, /* [in] Pointer to a value/resource. */    \
        void *data, /* [in,out] Callback function data. */                  \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Type for context callback function.
 */
typedef ARCHI_CONTEXT_CALLBACK_FUNC((*archi_context_callback_func_t));

/**
 * @brief Context callback.
 */
typedef struct archi_context_callback {
    archi_context_callback_func_t function; ///< Callback function.
    void *data; ///< Callback function data.
} archi_context_callback_t;

/**
 * @brief Context callback parameter identifier.
 */
#define ARCHI_CONTEXT_CALLBACK_PARAMETER        archi_parameter_context_callback

/**
 * @brief Context callback parameter declaration for function parameter lists.
 */
#define ARCHI_CONTEXT_CALLBACK_PARAMETER_DECL   \
    archi_context_callback_t ARCHI_CONTEXT_CALLBACK_PARAMETER

#endif // _ARCHI_CONTEXT_API_CALLBACK_TYP_H_

