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
 * @brief Types of context registry instructions.
 */

#pragma once
#ifndef _ARCHI_EXE_INSTRUCTION_TYP_H_
#define _ARCHI_EXE_INSTRUCTION_TYP_H_

#include "archi/ctx/interface.typ.h"

/**
 * @brief Type of context registry instructions.
 */
typedef enum archi_exe_registry_instr_type {
    ARCHI_EXE_REGISTRY_INSTR_NOOP = 0,     ///< No operation.

    ARCHI_EXE_REGISTRY_INSTR_INIT,          ///< Initialize a new context.
    ARCHI_EXE_REGISTRY_INSTR_FINAL,         ///< Finalize a context.

    ARCHI_EXE_REGISTRY_INSTR_SET_VALUE,     ///< Set context slot to pointer to a value.
    ARCHI_EXE_REGISTRY_INSTR_SET_CONTEXT,   ///< Set context slot to pointer to a source context.
    ARCHI_EXE_REGISTRY_INSTR_SET_SLOT,      ///< Set context slot to a source context slot.

    ARCHI_EXE_REGISTRY_INSTR_ACT,           ///< Perform a context action.
} archi_exe_registry_instr_type_t;

/**
 * @brief Context registry instruction base.
 */
typedef struct archi_exe_registry_instr_base {
    archi_exe_registry_instr_type_t type; ///< Instruction type.
    const char *key; ///< Key of the context.
} archi_exe_registry_instr_base_t;

struct archi_exe_registry_instr_list;

/**
 * @brief Context registry instruction list.
 */
typedef struct archi_exe_registry_instr_list {
    struct archi_exe_registry_instr_list *next; ///< Pointer to the next list node.

    archi_exe_registry_instr_base_t *instruction; ///< Instruction.
} archi_exe_registry_instr_list_t;

/*****************************************************************************/

/**
 * @brief Context registry instruction: initialize a new context.
 *
 * `interface_key` may be NULL, which is the same as providing
 * a key referring to an instance of `archi_context_parameters_interface`.
 *
 * `interface_key` may be an empty string, which is the same as providing
 * a key referring to an instance of `archi_context_pointer_interface`.
 */
typedef struct archi_exe_registry_instr_init {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *interface_key; ///< Key of the context interface.

    const char *dparams_key; ///< Key of the dynamic parameter list.
    const archi_parameter_list_t *sparams; ///< Static parameter list.
} archi_exe_registry_instr_init_t;

/**
 * @brief Context registry instruction: set context slot to pointer to a value.
 */
typedef struct archi_exe_registry_instr_set_value {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    archi_context_op_designator_t slot; ///< Slot designator.
    archi_pointer_t value; ///< Value to set.
} archi_exe_registry_instr_set_value_t;

/**
 * @brief Context registry instruction: set context slot to pointer to a source context.
 */
typedef struct archi_exe_registry_instr_set_context {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    archi_context_op_designator_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
} archi_exe_registry_instr_set_context_t;

/**
 * @brief Context registry instruction: set context slot to a source context slot.
 */
typedef struct archi_exe_registry_instr_set_slot {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    archi_context_op_designator_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
    archi_context_op_designator_t source_slot; ///< Source slot designator.
} archi_exe_registry_instr_set_slot_t;

/**
 * @brief Context registry instruction: invoke context action.
 */
typedef struct archi_exe_registry_instr_act {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    archi_context_op_designator_t action; ///< Action designator.

    const char *dparams_key; ///< Key of the dynamic parameter list.
    const archi_parameter_list_t *sparams; ///< Static parameter list.
} archi_exe_registry_instr_act_t;

#endif // _ARCHI_EXE_INSTRUCTION_TYP_H_

