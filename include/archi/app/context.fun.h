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
 * @brief Operations on contexts.
 */

#pragma once
#ifndef _ARCHI_APP_CONTEXT_FUN_H_
#define _ARCHI_APP_CONTEXT_FUN_H_

#include "archi/app/context.typ.h"

/**
 * @brief Initialize a context.
 *
 * @return Status code.
 */
archi_status_t
archi_context_initialize(
        archi_context_t *context, ///< [in,out] Initialized context.

        const struct archi_list_node_named_value *config ///< [in] Context configuration.
);

/**
 * @brief Finalize a context.
 */
void
archi_context_finalize(
        archi_context_t *context ///< [in,out] Context.
);

/**
 * @brief Set a context slot.
 *
 * @return Status code.
 */
archi_status_t
archi_context_set(
        archi_context_t *context, ///< [in,out] Context.

        const char *slot, ///< [in] Context slot.
        const archi_value_t *value ///< [in] Value to set.
);

/**
 * @brief Get a context slot.
 *
 * @return Status code.
 */
archi_status_t
archi_context_get(
        archi_context_t *context, ///< [in,out] Context.

        const char *slot, ///< [in] Context slot.
        archi_value_t *value ///< [out] Gotten value.
);

/**
 * @brief Perform a context assignment.
 *
 * Destination context slot must not be null.
 * Source context slot may be null.
 * In that case, the source context itself
 * is passed to the destination slot setter function.
 *
 * @return Status code.
 */
archi_status_t
archi_context_assign(
        archi_context_t *dest, ///< [in,out] Destination context.
        const char *dest_slot, ///< [in] Destination context slot.

        archi_context_t *src, ///< [in,out] Source context.
        const char *src_slot  ///< [in] Source context slot.
);

/**
 * @brief Invoke a context action.
 *
 * @return Status code.
 */
archi_status_t
archi_context_act(
        archi_context_t *context, ///< [in,out] Initialized context.

        const char *action, ///< [in] Action type.
        const struct archi_list_node_named_value *params ///< [in] Action parameters.
);

#endif // _ARCHI_APP_CONTEXT_FUN_H_

