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

#include "archipelago/context/interface.typ.h"

/**
 * @brief Type of context registry instructions.
 */
typedef enum archi_exe_registry_instr_type {
    ARCHI_EXE_REGISTRY_INSTR__NOOP = 0,              ///< No operation.

    /* Operations on registry elements */
    ARCHI_EXE_REGISTRY_INSTR__DELETE,                ///< Delete a context from registry.
    ARCHI_EXE_REGISTRY_INSTR__COPY,                  ///< Create a context alias.

    /* Initialization of contexts of built-in types */
    ARCHI_EXE_REGISTRY_INSTR__INIT_PARAMETERS,       ///< Initialize a new parameter list context.
    ARCHI_EXE_REGISTRY_INSTR__INIT_POINTER,          ///< Initialize a new pointer context.
    ARCHI_EXE_REGISTRY_INSTR__INIT_ARRAY,            ///< Initialize a new data pointer array context.

    /* Initialization of new contexts */
    ARCHI_EXE_REGISTRY_INSTR__INIT_FROM_CONTEXT,     ///< Initialize a new context using interface of an origin context.
    ARCHI_EXE_REGISTRY_INSTR__INIT_FROM_SLOT,        ///< Initialize a new context using interface from a context slot.

    /* Calling context slot setters */
    ARCHI_EXE_REGISTRY_INSTR__SET_TO_VALUE,          ///< Set context slot to pointer to a value.
    ARCHI_EXE_REGISTRY_INSTR__SET_TO_CONTEXT_DATA,   ///< Set context slot to pointer to a source context.
    ARCHI_EXE_REGISTRY_INSTR__SET_TO_CONTEXT_SLOT,   ///< Set context slot to a source context slot.

    /* Calling context actions */
    ARCHI_EXE_REGISTRY_INSTR__ACT,                   ///< Invoke a context action.
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
 * @brief Context registry instruction: delete a context from registry.
 */
typedef struct archi_exe_registry_instr__delete {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the context.
} archi_exe_registry_instr__delete_t;

/**
 * @brief Context registry instruction: create a context alias.
 */
typedef struct archi_exe_registry_instr__copy {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the new context alias.

    const char *original_key; ///< Original context key.
} archi_exe_registry_instr__copy_t;

/*****************************************************************************/

/**
 * @brief Context registry instruction: initialize a new parameter list context.
 */
typedef struct archi_exe_registry_instr__init_parameters {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the new context.

    const char *dparams_key; ///< Key of the dynamic parameter list.
    const archi_parameter_list_t *sparams; ///< Static parameter list.
} archi_exe_registry_instr__init_parameters_t;

/**
 * @brief Context registry instruction: initialize a new pointer context.
 */
typedef struct archi_exe_registry_instr__init_pointer {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the new context.

    archi_pointer_t value; ///< Value of the pointer.
} archi_exe_registry_instr__init_pointer_t;

/**
 * @brief Context registry instruction: initialize a new array context.
 */
typedef struct archi_exe_registry_instr__init_array {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the new context.

    size_t num_elements; ///< Number of elements in the array.
    archi_pointer_attributes_t flags; ///< Array flags.
} archi_exe_registry_instr__init_array_t;

/*****************************************************************************/

/**
 * @brief Context registry instruction: initialize a new context using interface of another context.
 */
typedef struct archi_exe_registry_instr__init_from_context {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the new context.

    const char *interface_origin_key; ///< Key of the interface origin context.

    const char *dparams_key; ///< Key of the dynamic parameter list.
    const archi_parameter_list_t *sparams; ///< Static parameter list.
} archi_exe_registry_instr__init_from_context_t;

/**
 * @brief Context registry instruction: initialize a new context using interface obtained from a context slot.
 */
typedef struct archi_exe_registry_instr__init_from_slot {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the new context.

    const char *interface_origin_key; ///< Key of the interface origin context.
    archi_context_slot_t interface_origin_slot; ///< Interface origin slot designator.

    const char *dparams_key; ///< Key of the dynamic parameter list.
    const archi_parameter_list_t *sparams; ///< Static parameter list.
} archi_exe_registry_instr__init_from_slot_t;

/*****************************************************************************/

/**
 * @brief Context registry instruction: set context slot to pointer to a value.
 */
typedef struct archi_exe_registry_instr__set_to_value {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the context.

    archi_context_slot_t slot; ///< Slot designator.
    archi_pointer_t value; ///< Value to set.
} archi_exe_registry_instr__set_to_value_t;

/**
 * @brief Context registry instruction: set context slot to pointer to a source context.
 */
typedef struct archi_exe_registry_instr__set_to_context_data {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the context.

    archi_context_slot_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
} archi_exe_registry_instr__set_to_context_data_t;

/**
 * @brief Context registry instruction: set context slot to a source context slot.
 */
typedef struct archi_exe_registry_instr__set_to_context_slot {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the context.

    archi_context_slot_t slot; ///< Slot designator.
    const char *source_key; ///< Key of the source context.
    archi_context_slot_t source_slot; ///< Source slot designator.
} archi_exe_registry_instr__set_to_context_slot_t;

/*****************************************************************************/

/**
 * @brief Context registry instruction: invoke context action.
 */
typedef struct archi_exe_registry_instr__act {
    archi_exe_registry_instr_base_t base; ///< Instruction base.
    const char *key; ///< Key of the context.

    archi_context_slot_t action; ///< Action designator.

    const char *dparams_key; ///< Key of the dynamic parameter list.
    const archi_parameter_list_t *sparams; ///< Static parameter list.
} archi_exe_registry_instr__act_t;

#endif // _ARCHI_EXE_INSTRUCTION_TYP_H_

