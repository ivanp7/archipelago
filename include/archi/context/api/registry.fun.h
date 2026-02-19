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
 * @brief Context registry operations.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_API_REGISTRY_FUN_H_
#define _ARCHI_CONTEXT_API_REGISTRY_FUN_H_

#include "archi/context/api/registry.typ.h"
#include "archi/context/api/handle.typ.h"
#include "archi/context/api/slot.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


/**
 * @brief Context registry operation: extract a context.
 *
 * @note This function returns NULL for non-context values stored in a registry.
 *
 * @return Context stored under the specified key, or NULL in case of failure.
 */
archi_context_t
archi_context_registry_get(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Context key.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: check if there a context stored under the specified key.
 *
 * @note This function returns false for non-context values stored in a registry.
 *
 * @return True if there is a context associated with the key, false otherwise.
 */
bool
archi_context_registry_contains(
        archi_context_t registry, ///< [in] Context registry.
        const char *key ///< [in] Context key.
);

/**
 * @brief Context registry operation: check if the specified key is available.
 *
 * @note This function returns false for if a key is associated with a non-context value.
 *
 * @return True if the key does not exist, false otherwise.
 */
bool
archi_context_registry_key_available(
        archi_context_t registry, ///< [in] Context registry.
        const char *key ///< [in] Context key.
);

/**
 * @brief Context registry operation: delete a context.
 */
void
archi_context_registry_delete(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Deleted context key.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: create a context alias.
 */
void
archi_context_registry_alias(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a created context alias.
        const char *original_key, ///< [in] Original context key.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: insert a context.
 */
void
archi_context_registry_insert(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a inserted context.
        archi_context_t context, ///< [in] Context to insert.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: create a new context.
 *
 * @note Tail of the list passed as `init_params.list` is modified during the operation
 * and restored to its original state before return.
 *
 * @return Created context, or NULL in case of failure.
 */
archi_context_t
archi_context_registry_create(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a created context.
        archi_rcpointer_t interface, ///< [in] Context interface.
        archi_context_registry_params_t init_params, ///< [in] Context initialization parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: create a new context (as another context).
 *
 * @note Tail of the list passed as `init_params.list` is modified during the operation
 * and restored to its original state before return.
 *
 * @return Created context, or NULL in case of failure.
 */
archi_context_t
archi_context_registry_create_as(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a created context.
        const char *sample_key, ///< [in] Key of a context with the required interface.
        archi_context_registry_params_t init_params, ///< [in] Context initialization parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: create a new context
 * (with interface obtained from a slot of another context).
 *
 * @note Tail of the list passed as `init_params.list` is modified during the operation
 * and restored to its original state before return.
 *
 * @return Created context, or NULL in case of failure.
 */
archi_context_t
archi_context_registry_create_from(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a created context.
        const char *source_key, ///< [in] Key of an interface source context.
        archi_context_slot_t source_slot, ///< [in] Source context slot.
        archi_context_registry_params_t init_params, ///< [in] Context initialization parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: invoke a context call.
 *
 * @note Tail of the list passed as `call_params.list` is modified during the operation
 * and restored to its original state before return.
 */
void
archi_context_registry_invoke(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a target context.
        archi_context_slot_t slot, ///< [in] Target context slot.
        archi_context_registry_params_t call_params, ///< [in] Context call parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: unassign a context slot.
 */
void
archi_context_registry_unassign(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a target context.
        archi_context_slot_t slot, ///< [in] Target context slot.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: assign a value to a context slot.
 */
void
archi_context_registry_assign(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a target context.
        archi_context_slot_t slot, ///< [in] Target context slot.
        bool weak_ref, ///< [in] Whether reference counter of the assigned value must be unset.
        archi_rcpointer_t value, ///< [in] Assigned value.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: assign a value to a context slot
 * (with the value obtained from a slot of another context).
 */
void
archi_context_registry_assign_slot(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a target context.
        archi_context_slot_t slot, ///< [in] Target context slot.
        bool weak_ref, ///< [in] Whether reference counter of the assigned value must be unset.
        const char *source_key, ///< [in] Key of a source context.
        archi_context_slot_t source_slot, ///< [in] Source context slot.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Context registry operation: assign a value to a context slot
 * (with the value obtained from a call of another context).
 */
void
archi_context_registry_assign_call(
        archi_context_t registry, ///< [in] Context registry.
        const char *key, ///< [in] Key of a target context.
        archi_context_slot_t slot, ///< [in] Target context slot.
        bool weak_ref, ///< [in] Whether reference counter of the assigned value must be unset.
        const char *source_key, ///< [in] Key of an interface source context.
        archi_context_slot_t source_slot, ///< [in] Source context slot.
        archi_context_registry_params_t source_call_params, ///< [in] Source context call parameters.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_CONTEXT_API_REGISTRY_FUN_H_

