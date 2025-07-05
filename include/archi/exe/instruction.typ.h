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
    ARCHI_EXE_REGISTRY_INSTR_NOOP = 0,              ///< No operation.

    /* Initialization of new contexts */
    ARCHI_EXE_REGISTRY_INSTR_INIT_FROM_CONTEXT,     ///< Initialize a new context using interface of a source context.
    ARCHI_EXE_REGISTRY_INSTR_INIT_FROM_SLOT,        ///< Initialize a new context using interface from a context slot.

    /* Initialization of auxiliary contexts */
    ARCHI_EXE_REGISTRY_INSTR_INIT_POINTER,          ///< Initialize a new pointer context.
    ARCHI_EXE_REGISTRY_INSTR_INIT_DATA_ARRAY,       ///< Initialize a new data pointer array context.
    ARCHI_EXE_REGISTRY_INSTR_INIT_FUNC_ARRAY,       ///< Initialize a new function pointer array context.

    ARCHI_EXE_REGISTRY_INSTR_COPY,                  ///< Create a context alias.

    /* Removing of contexts from registry */
    ARCHI_EXE_REGISTRY_INSTR_FINAL,                 ///< Finalize a context.

    /* Calling contexts setters */
    ARCHI_EXE_REGISTRY_INSTR_SET_TO_VALUE,          ///< Set context slot to pointer to a value.
    ARCHI_EXE_REGISTRY_INSTR_SET_TO_CONTEXT_DATA,   ///< Set context slot to pointer to a source context.
    ARCHI_EXE_REGISTRY_INSTR_SET_TO_CONTEXT_SLOT,   ///< Set context slot to a source context slot.

    /* Calling contexts actions */
    ARCHI_EXE_REGISTRY_INSTR_ACT,                   ///< Perform a context action.
} archi_exe_registry_instr_type_t;

/**
 * @brief Context registry instruction base.
 */
typedef struct archi_exe_registry_instr_base {
    archi_exe_registry_instr_type_t type; ///< Instruction type.
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
 * @brief Context registry instruction: initialize a new context using interface of another context.
 *
 * `source_key` may be NULL, which is the same as providing
 * a key referring to an instance of `archi_context_parameters_interface`.
 *
 * `source_key` may be an empty string, which is the same as providing
 * a key referring to an instance of `archi_context_pointer_interface`.
 */
typedef struct archi_exe_registry_instr_init_from_context {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the new context.

    const char *interface_source_key; ///< Key of the interface source context.

    const char *dparams_key; ///< Key of the dynamic parameter list.
    const archi_parameter_list_t *sparams; ///< Static parameter list.
} archi_exe_registry_instr_init_from_context_t;

/**
 * @brief Context registry instruction: initialize a new context using interface obtained from a context slot.
 */
typedef struct archi_exe_registry_instr_init_from_slot {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the new context.

    const char *interface_source_key; ///< Key of the interface source context.
    archi_context_slot_t interface_source_slot; ///< Interface source slot designator.

    const char *dparams_key; ///< Key of the dynamic parameter list.
    const archi_parameter_list_t *sparams; ///< Static parameter list.
} archi_exe_registry_instr_init_from_slot_t;

/**
 * @brief Context registry instruction: initialize a new pointer context.
 */
typedef struct archi_exe_registry_instr_init_pointer {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the new context.

    archi_pointer_t value; ///< Value of the pointer.
} archi_exe_registry_instr_init_pointer_t;

/**
 * @brief Context registry instruction: initialize a new array context.
 */
typedef struct archi_exe_registry_instr_init_array {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the new context.

    size_t num_elements; ///< Number of elements in the array.
    archi_pointer_flags_t flags; ///< Array flags.
} archi_exe_registry_instr_init_array_t;

/**
 * @brief Context registry instruction: create a context alias.
 */
typedef struct archi_exe_registry_instr_copy {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the new context alias.

    const char *original_key; ///< Original context key.
} archi_exe_registry_instr_copy_t;

/**
 * @brief Context registry instruction: finalize a context.
 */
typedef struct archi_exe_registry_instr_final {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the context.
} archi_exe_registry_instr_final_t;

/**
 * @brief Context registry instruction: set context slot to pointer to a value.
 */
typedef struct archi_exe_registry_instr_set_to_value {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the context.

    archi_context_slot_t slot; ///< Slot designator.
    archi_pointer_t value; ///< Value to set.
} archi_exe_registry_instr_set_to_value_t;

/**
 * @brief Context registry instruction: set context slot to pointer to a source context.
 */
typedef struct archi_exe_registry_instr_set_to_context_data {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the context.

    archi_context_slot_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
} archi_exe_registry_instr_set_to_context_data_t;

/**
 * @brief Context registry instruction: set context slot to a source context slot.
 */
typedef struct archi_exe_registry_instr_set_to_context_slot {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the context.

    archi_context_slot_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
    archi_context_slot_t source_slot; ///< Source slot designator.
} archi_exe_registry_instr_set_to_context_slot_t;

/**
 * @brief Context registry instruction: invoke context action.
 */
typedef struct archi_exe_registry_instr_act {
    archi_exe_registry_instr_base_t base; ///< Instruction base.

    const char *key; ///< Key of the context.

    archi_context_slot_t action; ///< Action designator.

    const char *dparams_key; ///< Key of the dynamic parameter list.
    const archi_parameter_list_t *sparams; ///< Static parameter list.
} archi_exe_registry_instr_act_t;

#endif // _ARCHI_EXE_INSTRUCTION_TYP_H_

