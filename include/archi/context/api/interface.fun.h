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
 * @brief Interface operations on contexts.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_API_INTERFACE_FUN_H_
#define _ARCHI_CONTEXT_API_INTERFACE_FUN_H_

#include "archi/context/api/handle.typ.h"
#include "archi/context/api/slot.typ.h"
#include "archi/context/api/callback.typ.h"
#include "archipelago/base/pointer.typ.h"
#include "archipelago/base/kvlist.typ.h"
#include "archipelago/base/error.typ.h"

#include <stdbool.h>

/**
 * @brief Extract context interface.
 *
 * @return Pointer the interface of the context.
 */
archi_rcpointer_t
archi_context_interface(
        archi_context_t context ///< [in] Context.
);

/**
 * @brief Extract context data.
 *
 * @note Reference counter associated with the returned pointer is that of the context itself,
 * not the counter returned by the initialization function.
 *
 * @return Pointer to the context data.
 */
archi_rcpointer_t
archi_context_data(
        archi_context_t context ///< [in] Context.
);

/*****************************************************************************/

/**
 * @brief Allocate and initialize a context.
 *
 * `ARCHI_CONTEXT_INIT_FUNC` contract violations are processed as follows:
 * If (1) is violated by setting non-zero error code, it is ignored.
 * If (2) is violated by setting zero error code, code for generic failure is set instead.
 *
 * @return Handle of a new initialized context.
 */
archi_context_t
archi_context_initialize(
        archi_rcpointer_t interface, ///< [in] Context interface.
        const archi_kvlist_rc_t *params, ///< [in] Initialization parameters.

        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Finalize and destroy a context.
 *
 * This function is equivalent to:
 * ```c
 * archi_reference_count_decrement(archi_context_data(context).ref_count);
 * ```
 *
 * The context is to be considered invalid after this call.
 */
void
archi_context_finalize(
        archi_context_t context ///< [in] Context.
);

/**
 * @brief Evaluate context slot without call semantics.
 *
 * This function calls the context evaluation function
 * with `call` set to false.
 *
 * Empty slot is processed in a special way. In this case,
 * `archi_context_data(context)` is provided to the callback,
 * and `eval_fn` function from the interface is not called.
 */
void
archi_context_get(
        archi_context_t context, ///< [in] Context.
        archi_context_slot_t slot, ///< [in] Slot designator.
        archi_context_callback_t callback, ///< [in] Output callback.

        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Evaluate context slot with call semantics.
 *
 * This function calls the context evaluation function
 * with `call` set to true.
 *
 * Empty slot is allowed and not processed in a special way.
 */
void
archi_context_call(
        archi_context_t context, ///< [in] Context.
        archi_context_slot_t slot, ///< [in] Slot designator.
        const archi_kvlist_rc_t *params, ///< [in] Call parameters.
        archi_context_callback_t callback, ///< [in] Output callback.

        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Set value to context slot.
 *
 * Setting empty slot is not allowed.
 */
void
archi_context_set(
        archi_context_t context, ///< [in] Context.
        archi_context_slot_t slot, ///< [in] Slot designator.
        archi_rcpointer_t value, ///< [in] Value to set.

        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/*****************************************************************************/

/**
 * @brief Get value from source context/slot and set it to destination context/slot.
 *
 * Destination slot cannot be empty.
 */
void
archi_context_set_from_get(
        archi_context_t context,   ///< [in] Destination context.
        archi_context_slot_t slot, ///< [in] Destination slot designator.

        archi_context_t src_context,   ///< [in] Source context.
        archi_context_slot_t src_slot, ///< [in] Source slot designator.
        bool src_no_refcount, ///< [in] Whether to reset reference counter of the source value.

        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Get call result from source context/slot and set it to destination context/slot.
 *
 * Destination slot cannot be empty.
 */
void
archi_context_set_from_call(
        archi_context_t context, ///< [in] Destination context.
        archi_context_slot_t slot, ///< [in] Destination slot designator.

        archi_context_t src_context, ///< [in] Source context.
        archi_context_slot_t src_slot, ///< [in] Source slot designator.
        const archi_kvlist_rc_t *src_params, ///< [in] Source call parameters.
        bool src_no_refcount, ///< [in] Whether to reset reference counter of the source value.

        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

#endif // _ARCHI_CONTEXT_API_INTERFACE_FUN_H_

