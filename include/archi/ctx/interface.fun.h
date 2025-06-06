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
 * @brief Interface operations on contexts.
 */

#pragma once
#ifndef _ARCHI_CTX_INTERFACE_FUN_H_
#define _ARCHI_CTX_INTERFACE_FUN_H_

#include "archi/ctx/interface.typ.h"

struct archi_context;

/**
 * @brief Pointer to context.
 */
typedef struct archi_context *archi_context_t;

/**
 * @brief Extract context interface.
 *
 * @return Pointer the interface of the context.
 */
archi_pointer_t
archi_context_interface(
        archi_context_t context ///< [in] Context.
);

/**
 * @brief Extract context data.
 *
 * @return Pointer wrapper to the context data.
 */
archi_pointer_t
archi_context_data(
        archi_context_t context ///< [in] Context.
);

/*****************************************************************************/

/**
 * @brief Allocate and initialize a context.
 *
 * @return A new initialized context.
 */
archi_context_t
archi_context_initialize(
        archi_pointer_t interface, ///< [in] Context interface.
        const archi_parameter_list_t *params, ///< [in] Initialization parameters.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Finalize and destroy a context.
 *
 * This is done forcibly, without considering the reference count state.
 */
void
archi_context_finalize(
        archi_context_t context ///< [in] Context.
);

/*****************************************************************************/

/**
 * @brief Get value from context slot.
 *
 * @return Context slot value.
 */
archi_pointer_t
archi_context_get_slot(
        archi_context_t context, ///< [in] Context.
        archi_context_slot_t slot, ///< [in] Slot designator.
        archi_status_t *code ///< [out] Status code.
);

/**
 * @brief Set context slot to an arbitrary value.
 *
 * @return Status code.
 */
archi_status_t
archi_context_set_slot(
        archi_context_t context, ///< [in] Context.
        archi_context_slot_t slot, ///< [in] Slot designator.
        archi_pointer_t value ///< [in] Value to set.
);

/**
 * @brief Copy value between context slots.
 *
 * @return Status code.
 */
archi_status_t
archi_context_copy_slot(
        archi_context_t context, ///< [in] Destination context.
        archi_context_slot_t slot, ///< [in] Destination slot designator.

        archi_context_t src_context, ///< [in] Source context.
        archi_context_slot_t src_slot ///< [in] Source slot designator.
);

/*****************************************************************************/

/**
 * @brief Invoke a context action.
 *
 * @return Status code.
 */
archi_status_t
archi_context_act(
        archi_context_t context, ///< [in] Initialized context.
        archi_context_slot_t action, ///< [in] Action designator.
        const archi_parameter_list_t *params ///< [in] Action parameters.
);

#endif // _ARCHI_CTX_INTERFACE_FUN_H_

