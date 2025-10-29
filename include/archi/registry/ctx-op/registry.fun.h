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
#ifndef _ARCHI_REGISTRY_CTX_OP_REGISTRY_FUN_H_
#define _ARCHI_REGISTRY_CTX_OP_REGISTRY_FUN_H_

#include "archi/context/api/operation.typ.h"

/**
 * @brief Context registry operation: delete a context.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__delete);

/**
 * @brief Context registry operation: create a context alias.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__alias);

/**
 * @brief Context registry operation: create a context using interface of another context.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_as);

/**
 * @brief Context registry operation: create a context using interface obtained from another context slot.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_from);

/**
 * @brief Context registry operation: invoke context call.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__call);

/**
 * @brief Context registry operation: set context slot to value.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__set);

/**
 * @brief Context registry operation: set context slot to value of another context slot.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__assign);

/**
 * @brief Context registry operation: set context slot to value of another context slot (without reference counter).
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__weak_assign);

/**
 * @brief Context registry operation: set context slot to result of another context call.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__assign_call);

/**
 * @brief Context registry operation: set context slot to result of another context call (without reference counter).
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__weak_assign_call);

/*****************************************************************************/

/**
 * @brief Context registry operation: create a parameter list context.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_parameters);

/**
 * @brief Context registry operation: create a pointer context (to value).
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_ptr_to_value);

/**
 * @brief Context registry operation: create a pointer context (to context slot).
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_ptr_to_context);

/**
 * @brief Context registry operation: create a pointer context (to context slot, without reference counter).
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_weak_ptr_to_context);

/**
 * @brief Context registry operation: create a data pointer array context.
 *
 * If `context` is NULL, prints `argument` contents to log using archi_print().
 * Log stream must be already locked.
 */
ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_dptr_array);

#endif // _ARCHI_REGISTRY_CTX_OP_REGISTRY_FUN_H_

