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
 * @brief Template of application context interface for simple C structures.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_CTX_STRUCT_FUN_H_
#define _ARCHI_CONTEXT_CTX_STRUCT_FUN_H_

#include "archi/context/api/interface.typ.h"

struct archi_struct_info;

/**
 * @brief Context initialization function: structure.
 *
 * Initialization parameters: structure fields.
 *
 * @note `struct_info` is presumed to be correct.
 *
 * @return Context data.
 */
archi_rcpointer_t*
archi_context_init__struct(
        const archi_kvlist_rc_t *params, ///< [in] Initialization parameters.
        ARCHI_ERROR_PARAMETER_DECL, ///< [out] Error.

        const struct archi_struct_info *struct_info ///< [in] Structure info.
);

/**
 * @brief Context finalization function: structure.
 */
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__struct);

/**
 * @brief Context evaluation function: structure.
 *
 * Getter slots: structure fields (without indices only).
 *
 * Calls: none.
 */
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__struct);

/**
 * @brief Context setter function: structure.
 *
 * Setter slots: structure fields (without indices only).
 */
ARCHI_CONTEXT_SET_FUNC(archi_context_set__struct);

/**
 * @brief Define context interface for a concrete structure type.
 */
#define ARCHI_CONTEXT_INTERFACE_DEFINE_STRUCT(postfix, struct_info) \
static ARCHI_CONTEXT_INIT_FUNC(archi_context_init__##postfix) {     \
    return archi_context_init__struct(                              \
            params, ARCHI_ERROR_PARAMETER, (struct_info));          \
}                                                                   \
const archi_context_interface_t                                     \
archi_context_interface__##postfix = {                              \
    .init_fn = archi_context_init__##postfix,                       \
    .final_fn = archi_context_final__struct,                        \
    .eval_fn = archi_context_eval__struct,                          \
    .set_fn = archi_context_set__struct,                            \
}

#endif // _ARCHI_CONTEXT_CTX_STRUCT_FUN_H_

