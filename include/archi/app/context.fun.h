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
 * @brief Context operations.
 */

#pragma once
#ifndef _ARCHI_APP_CONTEXT_FUN_H_
#define _ARCHI_APP_CONTEXT_FUN_H_

#include "archi/app/context.typ.h"

struct archi_context;

/**
 * @brief Extract context interface.
 *
 * @return Pointer the interface of the context.
 */
archi_pointer_t
archi_context_interface(
        struct archi_context *context ///< [in] Context.
);

/**
 * @brief Extract context data.
 *
 * @return Pointer wrapper to the context data.
 */
archi_pointer_t
archi_context_data(
        struct archi_context *context ///< [in] Context.
);

/*****************************************************************************/

/**
 * @brief Allocate and initialize a context.
 *
 * @return A new initialized context.
 */
struct archi_context*
archi_context_initialize(
        archi_pointer_t interface, ///< [in] Context interface.
        const archi_context_parameter_list_t *params, ///< [in] Initialization parameters.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Finalize and destroy a context.
 *
 * This is done forcibly, without considering the reference count state.
 */
void
archi_context_finalize(
        struct archi_context *context ///< [in] Context.
);

/*****************************************************************************/

/**
 * @brief Get value from context slot.
 *
 * @return Context slot value.
 */
archi_pointer_t
archi_context_get_slot(
        struct archi_context *context, ///< [in] Context.
        archi_context_op_designator_t slot, ///< [in] Slot designator.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Set context slot to an arbitrary value.
 *
 * @return Status code.
 */
archi_status_t
archi_context_set_slot(
        struct archi_context *context, ///< [in] Context.
        archi_context_op_designator_t slot, ///< [in] Slot designator.
        archi_pointer_t value ///< [in] Value to set.
);

/**
 * @brief Copy value between context slots.
 *
 * @return Status code.
 */
archi_status_t
archi_context_copy_slot(
        struct archi_context *context, ///< [in] Destination context.
        archi_context_op_designator_t slot, ///< [in] Destination slot designator.

        struct archi_context *src_context, ///< [in] Source context.
        archi_context_op_designator_t src_slot ///< [in] Source slot designator.
);

/*****************************************************************************/

/**
 * @brief Invoke a context action.
 *
 * @return Status code.
 */
archi_status_t
archi_context_act(
        struct archi_context *context, ///< [in] Initialized context.
        archi_context_op_designator_t action, ///< [in] Action designator.
        const archi_context_parameter_list_t *params ///< [in] Action parameters.
);

#endif // _ARCHI_APP_CONTEXT_FUN_H_

