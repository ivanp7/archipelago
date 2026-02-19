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
 * @brief Context registry constants.
 */

#pragma once
#ifndef _ARCHI_APP_REGISTRY_DEF_H_
#define _ARCHI_APP_REGISTRY_DEF_H_

/*****************************************************************************/
// Built-in contexts
/*****************************************************************************/

#define ARCHI_APP_REGISTRY_CONTEXT__REGISTRY        "archi.registry"        ///< Built-in context: the context registry itself.

#define ARCHI_APP_REGISTRY_CONTEXT__EXECUTABLE      "archi.executable"      ///< Built-in context: the library handle of the executable itself.
#define ARCHI_APP_REGISTRY_CONTEXT__INPUT_FILE      "archi.input_file"      ///< Built-in context: the current input file context.

#define ARCHI_APP_REGISTRY_CONTEXT__SIGNAL_HANDLER  "archi.signal_handler"  ///< Built-in context: the signal handler data.

/*****************************************************************************/
// Registry operation names
/*****************************************************************************/

#define ARCHI_APP_REGISTRY_OP__DELETE            "delete"            ///< Registry operation: delete a context.
#define ARCHI_APP_REGISTRY_OP__ALIAS             "alias"             ///< Registry operation: create a context alias.
#define ARCHI_APP_REGISTRY_OP__CREATE_AS         "create_as"         ///< Registry operation: create a new context using interface of another context.
#define ARCHI_APP_REGISTRY_OP__CREATE_FROM       "create_from"       ///< Registry operation: create a new context using interface obtained from a slot of another context.
#define ARCHI_APP_REGISTRY_OP__CREATE_PARAMS     "create_params"     ///< Registry operation: create a parameter list context.
#define ARCHI_APP_REGISTRY_OP__CREATE_PTR        "create_ptr"        ///< Registry operation: create a pointer context.
#define ARCHI_APP_REGISTRY_OP__CREATE_DPTR_ARRAY "create_dptr_array" ///< Registry operation: create a data pointer array context.
#define ARCHI_APP_REGISTRY_OP__INVOKE            "invoke"            ///< Registry operation: invoke context call.
#define ARCHI_APP_REGISTRY_OP__UNASSIGN          "unassign"          ///< Registry operation: unassign a context slot.
#define ARCHI_APP_REGISTRY_OP__ASSIGN            "assign"            ///< Registry operation: assign a value to a context slot.
#define ARCHI_APP_REGISTRY_OP__ASSIGN_SLOT       "assign_slot"       ///< Registry operation: assign context slot to another context slot.
#define ARCHI_APP_REGISTRY_OP__ASSIGN_SLOT_WEAK  "assign_slot_weak"  ///< Registry operation: assign context slot to another context slot (unsetting reference counter).
#define ARCHI_APP_REGISTRY_OP__ASSIGN_CALL       "assign_call"       ///< Registry operation: set context slot to result of another context call.
#define ARCHI_APP_REGISTRY_OP__ASSIGN_CALL_WEAK  "assign_call_weak"  ///< Registry operation: set context slot to result of another context call (unsetting reference counter).

#endif // _ARCHI_APP_REGISTRY_DEF_H_

