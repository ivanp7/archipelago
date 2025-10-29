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
 * @brief Keys of the built-in contexts in the context registry.
 */

#pragma once
#ifndef _ARCHI_EXE_REGISTRY_DEF_H_
#define _ARCHI_EXE_REGISTRY_DEF_H_

/*****************************************************************************/
// Built-in contexts
/*****************************************************************************/

#define ARCHI_EXE_REGISTRY_CONTEXT__REGISTRY        "archi.registry"        ///< Built-in context: the context registry itself.
#define ARCHI_EXE_REGISTRY_CONTEXT__OPERATIONS      "archi.operations"      ///< Built-in context: the context registry operations.

#define ARCHI_EXE_REGISTRY_CONTEXT__EXECUTABLE      "archi.executable"      ///< Built-in context: the library handle of the executable itself.
#define ARCHI_EXE_REGISTRY_CONTEXT__INPUT_FILE      "archi.input_file"      ///< Built-in context: the current input file context.

#define ARCHI_EXE_REGISTRY_CONTEXT__SIGNAL_HANDLER  "archi.signal_handler"  ///< Built-in context: the signal handler data.

/*****************************************************************************/
// Built-in registry operations
/*****************************************************************************/

#define ARCHI_EXE_REGISTRY_OPERATION__DELETE            "delete"            ///< Registry operation: delete a context.
#define ARCHI_EXE_REGISTRY_OPERATION__ALIAS             "alias"             ///< Registry operation: create a context alias.
#define ARCHI_EXE_REGISTRY_OPERATION__CREATE_AS         "create_as"         ///< Registry operation: create a context using interface of another context.
#define ARCHI_EXE_REGISTRY_OPERATION__CREATE_FROM       "create_from"       ///< Registry operation: create a context using interface obtained from another context data/slot.
#define ARCHI_EXE_REGISTRY_OPERATION__CALL              "call"              ///< Registry operation: invoke context call.
#define ARCHI_EXE_REGISTRY_OPERATION__SET               "set"               ///< Registry operation: set context slot to value.
#define ARCHI_EXE_REGISTRY_OPERATION__ASSIGN            "assign"            ///< Registry operation: assign context slot to another context slot.
#define ARCHI_EXE_REGISTRY_OPERATION__WEAK_ASSIGN       "weak_assign"       ///< Registry operation: assign context slot to another context slot (without reference counter).
#define ARCHI_EXE_REGISTRY_OPERATION__ASSIGN_CALL       "assign_call"       ///< Registry operation: set context slot to result of another context call.
#define ARCHI_EXE_REGISTRY_OPERATION__WEAK_ASSIGN_CALL  "weak_assign_call"  ///< Registry operation: set context slot to result of another context call (without reference counter).


#define ARCHI_EXE_REGISTRY_OPERATION__CREATE_PARAMETERS             "create_parameters"             ///< Registry operation: create a parameters list context.
#define ARCHI_EXE_REGISTRY_OPERATION__CREATE_PTR_TO_VALUE           "create_ptr_to_value"           ///< Registry operation: create a pointer context (to value).
#define ARCHI_EXE_REGISTRY_OPERATION__CREATE_PTR_TO_CONTEXT         "create_ptr_to_context"         ///< Registry operation: create a pointer context (to context slot).
#define ARCHI_EXE_REGISTRY_OPERATION__CREATE_WEAK_PTR_TO_CONTEXT    "create_weak_ptr_to_context"    ///< Registry operation: create a pointer context (to context slot, without reference counter).
#define ARCHI_EXE_REGISTRY_OPERATION__CREATE_DPTR_ARRAY             "create_dptr_array"             ///< Registry operation: create a data pointer array context.

#endif // _ARCHI_EXE_REGISTRY_DEF_H_

