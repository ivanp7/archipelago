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
 * @brief The context interface type.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_API_INTERFACE_TYP_H_
#define _ARCHI_CONTEXT_API_INTERFACE_TYP_H_

#include "archi/context/api/slot.typ.h"
#include "archi/context/api/callback.typ.h"
#include "archi_base/pointer.typ.h"
#include "archi_base/kvlist.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


/**
 * @brief Declare/define context initialization function.
 *
 * This function is intended for initializing/allocating resources of a context.
 *
 * Contract:
 * (1) return non-NULL pointer and zero error code on success;
 * (2) return NULL pointer and non-zero error code on failure.
 *
 * @return Pointer to context data.
 */
#define ARCHI_CONTEXT_INIT_FUNC(func_name)  archi_rcpointer_t* func_name(       \
        const archi_krcvlist_t *params, /* [in] Initialization parameters. */   \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Context initialization function type.
 */
typedef ARCHI_CONTEXT_INIT_FUNC((*archi_context_init_func_t));

/**
 * @brief Declare/define context finalization function.
 *
 * This function is intended for finalizing/releasing resources of a context.
 */
#define ARCHI_CONTEXT_FINAL_FUNC(func_name) void func_name(                     \
        archi_rcpointer_t *context) /* [in] Context data. */

/**
 * @brief Context finalization function type.
 */
typedef ARCHI_CONTEXT_FINAL_FUNC((*archi_context_final_func_t));

/**
 * @brief Declare/define context slot evaluation function.
 *
 * This function is intended for performing computation with potential state changes,
 * and/or retrieving pointers from a context.
 *
 * Reference counter of the output value pointer may be set to
 * `ARCHI_CONTEXT_REF_COUNT` or `ARCHI_CONTEXT_INTERFACE_REF_COUNT`.
 * These special values will be automatically replaced with
 * a context reference counter or a interface reference counter, respectively.
 *
 * If `call` is false, the following conditions apply:
 * - `slot` is non-empty;
 * - `params` is NULL.
 *
 * Contract:
 * (1) if `call` is false, use ARCHI_CONTEXT_YIELD() exactly once, or signal error;
 * (2) if `call` is true, use ARCHI_CONTEXT_YIELD() at most once, or signal error.
 */
#define ARCHI_CONTEXT_EVAL_FUNC(func_name)   void func_name(                    \
        archi_rcpointer_t *context, /* [in] Context data. */                    \
        archi_context_slot_t slot, /* [in] Slot designator. */                  \
        bool call, /* [in] Whether call semantics are used. */                  \
        const archi_krcvlist_t *params, /* [in] Call parameters. */             \
        ARCHI_CONTEXT_CALLBACK_PARAMETER_DECL, /* [in] Output callback. */      \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Context slot evaluation function type.
 */
typedef ARCHI_CONTEXT_EVAL_FUNC((*archi_context_eval_func_t));

/**
 * @brief Declare/define context slot setter function.
 *
 * `slot` is always non-empty (either `slot.name` is non-empty or `slot.num_indices` is non-zero).
 *
 * If `unset` is true, the following conditions apply:
 * - `value` is empty (default-initialized).
 *
 * This function is intended for accepting pointers into a context.
 */
#define ARCHI_CONTEXT_SET_FUNC(func_name)   void func_name(                     \
        archi_rcpointer_t *context, /* [in] Context data. */                    \
        archi_context_slot_t slot, /* [in] Slot designator. */                  \
        bool unset, /* [in] Whether slot value is being unset. */               \
        archi_rcpointer_t value, /* [in] Value to set. */                       \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Context slot setter function type.
 */
typedef ARCHI_CONTEXT_SET_FUNC((*archi_context_set_func_t));

/*****************************************************************************/

/**
 * @brief Context interface functions.
 */
typedef struct archi_context_interface {
    archi_context_init_func_t init_fn;   ///< Context initialization function.
    archi_context_final_func_t final_fn; ///< Context finalization function.

    archi_context_eval_func_t eval_fn; ///< Context slot evaluation function.
    archi_context_set_func_t set_fn;   ///< Context slot setter function.
} archi_context_interface_t;

#endif // _ARCHI_CONTEXT_API_INTERFACE_TYP_H_

