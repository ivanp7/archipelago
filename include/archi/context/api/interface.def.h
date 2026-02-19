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
 * @brief Macros and constants for contexts.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_API_INTERFACE_DEF_H_
#define _ARCHI_CONTEXT_API_INTERFACE_DEF_H_

#include "archi/context/api/callback.typ.h"
#include "archi_base/ref_count.typ.h"


/**
 * @brief Special reference counter placeholder used for output values in context getters
 * (placeholder for context reference counter).
 */
#define ARCHI_CONTEXT_REF_COUNT             ((archi_reference_count_t)-1)

/**
 * @brief Special reference counter placeholder used for output values in context getters
 * (placeholder for context interface reference counter).
 */
#define ARCHI_CONTEXT_INTERFACE_REF_COUNT   ((archi_reference_count_t)-2)

/**
 * @brief Macro for checking if a callback function is not provided.
 */
#define ARCHI_CONTEXT_NO_CALLBACK() \
    (ARCHI_CONTEXT_CALLBACK_PARAMETER.function == NULL)

/**
 * @brief Macro for calling a context callback.
 *
 * @note This macro is to be used in functions that include
 * ARCHI_CONTEXT_CALLBACK_PARAMETER_DECL in their parameter list,
 * such as functions defined with ARCHI_CONTEXT_EVAL_FUNC().
 */
#define ARCHI_CONTEXT_YIELD(value)  do {                    \
    if (!ARCHI_CONTEXT_NO_CALLBACK())                       \
        ARCHI_CONTEXT_CALLBACK_PARAMETER.function((value),  \
                ARCHI_CONTEXT_CALLBACK_PARAMETER.data,      \
                ARCHI_ERROR_PARAM);                         \
} while (0)

#endif // _ARCHI_CONTEXT_API_INTERFACE_DEF_H_

