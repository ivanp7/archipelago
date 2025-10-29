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
 * @brief Data for context registry operations.
 */

#pragma once
#ifndef _ARCHI_REGISTRY_CTX_OP_REGISTRY_TYP_H_
#define _ARCHI_REGISTRY_CTX_OP_REGISTRY_TYP_H_

#include "archi/context/api/slot.typ.h"
#include "archipelago/base/kvlist.typ.h"

/**
 * @brief Key-slot pair for context registry operation data.
 */
typedef struct archi_context_registry_op_data_key_slot {
    const char *key; ///< Key of a context.
    archi_context_slot_t slot; ///< Slot designator.
} archi_context_registry_op_data_key_slot_t;

/**
 * @brief Parameter list description for context registry operation data.
 */
typedef struct archi_context_registry_op_data_params {
    const char *list_key; ///< Key of a parameter list context (dynamic parameter list).
    const archi_kvlist_t *list; ///< Static parameter list.
} archi_context_registry_op_data_params_t;

/**
 * @brief Context initialization info for context registry operation data.
 */
typedef struct archi_context_registry_op_data_init_info {
    const char *key; ///< Key of the created context.
    archi_context_registry_op_data_params_t init_params; ///< Context initialization parameters.
} archi_context_registry_op_data_init_info_t;

/*****************************************************************************/

/**
 * @brief Context registry operation data: delete a context.
 */
typedef struct archi_context_registry_op_data__delete {
    const char *key; ///< Key of the deleted context.
} archi_context_registry_op_data__delete_t;

/**
 * @brief Context registry operation data: create a context alias.
 */
typedef struct archi_context_registry_op_data__alias {
    const char *alias_key; ///< New alias key of the context.
    const char *origin_key; ///< Key of the origin context.
} archi_context_registry_op_data__alias_t;

/**
 * @brief Context registry operation data: create a context using interface of another context.
 */
typedef struct archi_context_registry_op_data__create_as {
    archi_context_registry_op_data_init_info_t context; ///< Context creation info.
    const char *instance_key; ///< Key of a context with the required interface.
} archi_context_registry_op_data__create_as_t;

/**
 * @brief Context registry operation data: create a context using interface obtained from another context slot.
 */
typedef struct archi_context_registry_op_data__create_from {
    archi_context_registry_op_data_init_info_t context; ///< Context creation info.
    archi_context_registry_op_data_key_slot_t origin; ///< Origin of the required interface.
} archi_context_registry_op_data__create_from_t;

/**
 * @brief Context registry operation data: invoke context call.
 */
typedef struct archi_context_registry_op_data__call {
    archi_context_registry_op_data_key_slot_t target; ///< Target.
    archi_context_registry_op_data_params_t call_params; ///< Call parameters.
} archi_context_registry_op_data__call_t;

/**
 * @brief Context registry operation data: set context slot to value.
 */
typedef struct archi_context_registry_op_data__set {
    archi_context_registry_op_data_key_slot_t target; ///< Target.
    archi_pointer_t value; ///< Value to set.
} archi_context_registry_op_data__set_t;

/**
 * @brief Context registry operation data: set context slot to value of another context slot.
 */
typedef struct archi_context_registry_op_data__assign {
    archi_context_registry_op_data_key_slot_t target; ///< Target.
    archi_context_registry_op_data_key_slot_t source; ///< Source.
} archi_context_registry_op_data__assign_t;

/**
 * @brief Context registry operation data: set context slot to result of another context call.
 */
typedef struct archi_context_registry_op_data__assign_call {
    archi_context_registry_op_data_key_slot_t target; ///< Target.
    archi_context_registry_op_data_key_slot_t source; ///< Source.
    archi_context_registry_op_data_params_t call_params; ///< Call parameters.
} archi_context_registry_op_data__assign_call_t;

/*****************************************************************************/

/**
 * @brief Context registry operation data: create a parameter list context.
 */
typedef archi_context_registry_op_data_init_info_t archi_context_registry_op_data__create_parameters_t;

/**
 * @brief Type of context interface for pointers.
 */
typedef enum archi_context_registry_op_data_ptr_type {
    ARCHI_CONTEXT_REGISTRY_PTR_TYPE__GENERIC = 0, ///< Interface for pointers of any type.
    ARCHI_CONTEXT_REGISTRY_PTR_TYPE__DATA,        ///< Interface for data pointers.
    ARCHI_CONTEXT_REGISTRY_PTR_TYPE__TRANSP_DATA, ///< Interface for transparent data pointers.
    ARCHI_CONTEXT_REGISTRY_PTR_TYPE__OPAQUE_DATA, ///< Interface for opaque data pointers.
    ARCHI_CONTEXT_REGISTRY_PTR_TYPE__FUNCTION,    ///< Interface for function pointers.
} archi_context_registry_op_data_ptr_type_t;

/**
 * @brief Context registry operation data: create a pointer context (to value).
 */
typedef struct archi_context_registry_op_data__create_ptr_to_value {
    const char *key; ///< Key of the created context.
    archi_pointer_t value; ///< Value to point to.
    archi_context_registry_op_data_ptr_type_t type; ///< Pointer interface type.
    const archi_kvlist_t *init_params_list; ///< Other pointer context initialization parameters.
} archi_context_registry_op_data__create_ptr_to_value_t;

/**
 * @brief Context registry operation data: create a pointer context (to context slot).
 */
typedef struct archi_context_registry_op_data__create_ptr_to_context {
    const char *key; ///< Key of the created context.
    archi_context_registry_op_data_key_slot_t pointee; ///< Pointee.
    archi_context_registry_op_data_ptr_type_t type; ///< Pointer interface type.
    const archi_kvlist_t *init_params_list; ///< Other pointer context initialization parameters.
} archi_context_registry_op_data__create_ptr_to_context_t;

/**
 * @brief Context registry operation data: create a data pointer array context.
 */
typedef struct archi_context_registry_op_data__create_dptr_array {
    const char *key; ///< Key of the created context.
    size_t length; ///< Length of the created array.
} archi_context_registry_op_data__create_dptr_array_t;

#endif // _ARCHI_REGISTRY_CTX_OP_REGISTRY_TYP_H_

