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
 * @brief Types of application context registry instructions.
 */

#pragma once
#ifndef _ARCHI_APP_REGISTRY_TYP_H_
#define _ARCHI_APP_REGISTRY_TYP_H_

#include "archi/app/context.typ.h"

/**
 * @brief Type of application instructions.
 */
typedef enum archi_context_registry_instr_type {
    ARCHI_CONTEXT_REGISTRY_INSTR_HALT = -1,    ///< Instruction list terminator.
    ARCHI_CONTEXT_REGISTRY_INSTR_NOOP = 0,     ///< No operation.

    ARCHI_CONTEXT_REGISTRY_INSTR_INIT_STATIC,  ///< Initialize a new context (using static parameter list).
    ARCHI_CONTEXT_REGISTRY_INSTR_INIT_DYNAMIC, ///< Initialize a new context (using dynamic parameter list).
    ARCHI_CONTEXT_REGISTRY_INSTR_FINAL,        ///< Finalize a context.

    ARCHI_CONTEXT_REGISTRY_INSTR_SET_VALUE,    ///< Set context slot to pointer to a value.
    ARCHI_CONTEXT_REGISTRY_INSTR_SET_CONTEXT,  ///< Set context slot to pointer to a source context.
    ARCHI_CONTEXT_REGISTRY_INSTR_SET_SLOT,     ///< Set context slot to a source context slot.

    ARCHI_CONTEXT_REGISTRY_INSTR_ACT_STATIC,   ///< Perform a context action (using static parameter list).
    ARCHI_CONTEXT_REGISTRY_INSTR_ACT_DYNAMIC,  ///< Perform a context action (using dynamic parameter list).
} archi_context_registry_instr_type_t;

/**
 * @brief Application context registry instruction base.
 */
typedef struct archi_context_registry_instr_base {
    archi_context_registry_instr_type_t type; ///< Instruction type.
    const char *key; ///< Key of the context.
} archi_context_registry_instr_base_t;

struct archi_context_registry_instr_list;

/**
 * @brief Application context registry instruction list.
 */
typedef struct archi_context_registry_instr_list {
    struct archi_context_registry_instr_list *next; ///< Pointer to the next list node.

    archi_context_registry_instr_base_t *instruction; ///< Instruction.
} archi_context_registry_instr_list_t;

/*****************************************************************************/

/**
 * @brief Application context registry instruction: initialize a new context.
 *
 * `interface_key` may be NULL, which is the same as providing
 * a key referring to an instance of `archi_context_parameters_interface`.
 *
 * `interface_key` may be an empty string, which is the same as providing
 * a key referring to an instance of `archi_context_pointer_interface`.
 */
typedef struct archi_context_registry_instr_init {
    archi_context_registry_instr_base_t base; ///< Instruction base.

    const char *interface_key; ///< Key of the context interface.

    union {
        const char *dparams_key; ///< Key of the dynamic parameter list.
        const archi_context_parameter_list_t *sparams; ///< Static parameter list.
    };
} archi_context_registry_instr_init_t;

/**
 * @brief Application context registry instruction: set context slot to pointer to a value.
 */
typedef struct archi_context_registry_instr_set_value {
    archi_context_registry_instr_base_t base; ///< Instruction base.

    archi_context_op_designator_t slot; ///< Slot designator.
    archi_pointer_t value; ///< Value to set.
} archi_context_registry_instr_set_value_t;

/**
 * @brief Application context registry instruction: set context slot to pointer to a source context.
 */
typedef struct archi_context_registry_instr_set_context {
    archi_context_registry_instr_base_t base; ///< Instruction base.

    archi_context_op_designator_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
} archi_context_registry_instr_set_context_t;

/**
 * @brief Application context registry instruction: set context slot to a source context slot.
 */
typedef struct archi_context_registry_instr_set_slot {
    archi_context_registry_instr_base_t base; ///< Instruction base.

    archi_context_op_designator_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
    archi_context_op_designator_t source_slot; ///< Source slot designator.
} archi_context_registry_instr_set_slot_t;

/**
 * @brief Application context registry instruction for context action.
 */
typedef struct archi_context_registry_instr_act {
    archi_context_registry_instr_base_t base; ///< Instruction base.

    archi_context_op_designator_t action; ///< Action designator.

    union {
        const char *dparams_key; ///< Key of the dynamic parameter list.
        const archi_context_parameter_list_t *sparams; ///< Static parameter list.
    };
} archi_context_registry_instr_act_t;

#endif // _ARCHI_APP_REGISTRY_TYP_H_

