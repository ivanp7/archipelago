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
 * @brief Types used to describe context registry operations in application input files.
 */

#pragma once
#ifndef _ARCHI_APP_REGISTRY_TYP_H_
#define _ARCHI_APP_REGISTRY_TYP_H_

#include "archi/context/api/handle.typ.h"
#include "archi/context/api/slot.typ.h"
#include "archi_base/kvlist.typ.h"
#include "archi_base/ref_count.typ.h"
#include "archi_base/error.typ.h"


/**
 * @brief Signature of a context registry operation function.
 */
#define ARCHI_APP_REGISTRY_OPERATION_FUNC(func_name) void func_name(                \
        archi_context_t registry, /* [in] Context registry. */                      \
        archi_pointer_t data, /* [in] Operation data. */                            \
        archi_reference_count_t ref_count, /* [in] Reference counter for data. */   \
        ARCHI_ERROR_PARAM_DECL) /* [out] Error. */

/**
 * @brief Context registry operation function type.
 */
typedef ARCHI_APP_REGISTRY_OPERATION_FUNC((*archi_app_registry_operation_func_t));

/**
 * @brief Context registry operation description.
 */
typedef struct archi_app_registry_operation {
    const char *name; ///< Operation name.
    archi_app_registry_operation_func_t function; ///< Operation function.
} archi_app_registry_operation_t;

/*****************************************************************************/

/**
 * @brief Parameter list description for context registry operation data.
 */
typedef struct archi_app_registry_op_data_params {
    const char *context_key; ///< Key of a parameter list context (dynamic parameter list).
    const archi_kvlist_t *list; ///< Static parameter list.
} archi_app_registry_op_data_params_t;

/*****************************************************************************/

/**
 * @brief Registry operation data: delete a context.
 */
typedef struct archi_app_registry_op_data__delete {
    const char *key; ///< Key of the deleted context.
} archi_app_registry_op_data__delete_t;

/**
 * @brief Registry operation data: create a context alias.
 */
typedef struct archi_app_registry_op_data__alias {
    const char *key; ///< Key of a created context alias.
    const char *original_key; ///< Key of the original context.
} archi_app_registry_op_data__alias_t;

/**
 * @brief Registry operation data: create a context using interface of another context.
 */
typedef struct archi_app_registry_op_data__create_as {
    const char *key; ///< Key of the created context.
    const char *sample_key; ///< Key of a context with the required interface.
    archi_app_registry_op_data_params_t init_params; ///< Context initialization parameters.
} archi_app_registry_op_data__create_as_t;

/**
 * @brief Registry operation data: create a context using interface obtained from another context slot.
 */
typedef struct archi_app_registry_op_data__create_from {
    const char *key; ///< Key of the created context.
    const char *source_key; ///< Key of a interface source context.
    archi_context_slot_t source_slot; ///< Source context slot.
    archi_app_registry_op_data_params_t init_params; ///< Context initialization parameters.
} archi_app_registry_op_data__create_from_t;

/**
 * @brief Registry operation data: create a parameter list context.
 */
typedef struct archi_app_registry_op_data__create_params {
    const char *key; ///< Key of the created context.
    archi_app_registry_op_data_params_t params; ///< List of parameters.
} archi_app_registry_op_data__create_params_t;

/**
 * @brief Registry operation data: create a pointer context.
 */
typedef struct archi_app_registry_op_data__create_ptr {
    const char *key; ///< Key of the created context.
    archi_pointer_t pointee; ///< Pointee.
} archi_app_registry_op_data__create_ptr_t;

/**
 * @brief Registry operation data: create a data pointer array context.
 */
typedef struct archi_app_registry_op_data__create_dptr_array {
    const char *key; ///< Key of the created context.
    size_t length; ///< Length of the created array.
} archi_app_registry_op_data__create_dptr_array_t;

/**
 * @brief Registry operation data: invoke context call.
 */
typedef struct archi_app_registry_op_data__invoke {
    const char *key; ///< Key of the target context.
    archi_context_slot_t slot; ///< Target context slot.
    archi_app_registry_op_data_params_t call_params; ///< Context call parameters.
} archi_app_registry_op_data__invoke_t;

/**
 * @brief Registry operation data: unset context slot.
 */
typedef struct archi_app_registry_op_data__unassign {
    const char *key; ///< Key of the target context.
    archi_context_slot_t slot; ///< Target context slot.
} archi_app_registry_op_data__unassign_t;

/**
 * @brief Registry operation data: set context slot to value.
 */
typedef struct archi_app_registry_op_data__assign {
    const char *key; ///< Key of the target context.
    archi_context_slot_t slot; ///< Target context slot.
    archi_pointer_t value; ///< Value to set.
} archi_app_registry_op_data__assign_t;

/**
 * @brief Registry operation data: set context slot to value of another context slot.
 */
typedef struct archi_app_registry_op_data__assign_slot {
    const char *key; ///< Key of the target context.
    archi_context_slot_t slot; ///< Target context slot.
    const char *source_key; ///< Key of the source context.
    archi_context_slot_t source_slot; ///< Source context slot.
} archi_app_registry_op_data__assign_slot_t;

/**
 * @brief Registry operation data: set context slot to value of another context call.
 */
typedef struct archi_app_registry_op_data__assign_call {
    const char *key; ///< Key of the target context.
    archi_context_slot_t slot; ///< Target context slot.
    const char *source_key; ///< Key of the source context.
    archi_context_slot_t source_slot; ///< Source context slot.
    archi_app_registry_op_data_params_t source_call_params; ///< Source context call parameters.
} archi_app_registry_op_data__assign_call_t;

#endif // _ARCHI_APP_REGISTRY_TYP_H_

