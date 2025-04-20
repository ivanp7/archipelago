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
 * @brief Types for application configuration instructions.
 */

#pragma once
#ifndef _ARCHI_APP_INSTRUCTION_TYP_H_
#define _ARCHI_APP_INSTRUCTION_TYP_H_

#include "archi/app/context.typ.h"

/**
 * @brief Type of application instructions.
 */
typedef enum archi_app_instruction_type {
    ARCHI_APP_INSTRUCTION_HALT = -1,    ///< Instruction list terminator.
    ARCHI_APP_INSTRUCTION_NOOP = 0,     ///< No operation.

    ARCHI_APP_INSTRUCTION_INIT_STATIC,  ///< Initialize a new context (using static parameter list).
    ARCHI_APP_INSTRUCTION_INIT_DYNAMIC, ///< Initialize a new context (using dynamic parameter list).
    ARCHI_APP_INSTRUCTION_FINAL,        ///< Finalize a context.

    ARCHI_APP_INSTRUCTION_SET_VALUE,    ///< Set context slot to pointer to a value.
    ARCHI_APP_INSTRUCTION_SET_CONTEXT,  ///< Set context slot to pointer to a source context.
    ARCHI_APP_INSTRUCTION_SET_SLOT,     ///< Set context slot to a source context slot.

    ARCHI_APP_INSTRUCTION_ACT_STATIC,   ///< Perform a context action (using static parameter list).
    ARCHI_APP_INSTRUCTION_ACT_DYNAMIC,  ///< Perform a context action (using dynamic parameter list).
} archi_app_instruction_type_t;

/**
 * @brief Application instruction base.
 */
typedef struct archi_app_instruction_base {
    archi_app_instruction_type_t type; ///< Instruction type.
    const char *key; ///< Key of the context.
} archi_app_instruction_base_t;

struct archi_app_instruction_list;

/**
 * @brief Application instruction list.
 */
typedef struct archi_app_instruction_list {
    struct archi_app_instruction_list *next; ///< Pointer to the next list node.

    archi_app_instruction_base_t *instruction; ///< Instruction.
} archi_app_instruction_list_t;

/*****************************************************************************/

/**
 * @brief Application instruction: initialize a new context.
 *
 * `interface_key` may be NULL, which is the same as providing
 * a key referring to an instance of `archi_context_parameters_interface`.
 *
 * `interface_key` may be an empty string, which is the same as providing
 * a key referring to an instance of `archi_context_pointer_interface`.
 */
typedef struct archi_app_instruction_init {
    archi_app_instruction_base_t base; ///< Instruction base.

    const char *interface_key; ///< Key of the context interface.

    union {
        const char *dparams_key; ///< Key of the dynamic parameter list.
        const archi_context_parameter_list_t *sparams; ///< Static parameter list.
    };
} archi_app_instruction_init_t;

/**
 * @brief Application instruction: set context slot to pointer to a value.
 */
typedef struct archi_app_instruction_set_value {
    archi_app_instruction_base_t base; ///< Instruction base.

    archi_context_op_designator_t slot; ///< Slot designator.
    archi_pointer_t value; ///< Value to set.
} archi_app_instruction_set_value_t;

/**
 * @brief Application instruction: set context slot to pointer to a source context.
 */
typedef struct archi_app_instruction_set_context {
    archi_app_instruction_base_t base; ///< Instruction base.

    archi_context_op_designator_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
} archi_app_instruction_set_context_t;

/**
 * @brief Application instruction: set context slot to a source context slot.
 */
typedef struct archi_app_instruction_set_slot {
    archi_app_instruction_base_t base; ///< Instruction base.

    archi_context_op_designator_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
    archi_context_op_designator_t source_slot; ///< Source slot designator.
} archi_app_instruction_set_slot_t;

/**
 * @brief Application instruction for context action.
 */
typedef struct archi_app_instruction_act {
    archi_app_instruction_base_t base; ///< Instruction base.

    archi_context_op_designator_t action; ///< Action designator.

    union {
        const char *dparams_key; ///< Key of the dynamic parameter list.
        const archi_context_parameter_list_t *sparams; ///< Static parameter list.
    };
} archi_app_instruction_act_t;

#endif // _ARCHI_APP_INSTRUCTION_TYP_H_

