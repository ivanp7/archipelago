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
#ifndef _ARCHI_APP_REGISTRY_FUN_H_
#define _ARCHI_APP_REGISTRY_FUN_H_

#include "archi_app/registry.typ.h"


ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__delete);               ///< Registry operation: delete a context.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__alias);                ///< Registry operation: create a context alias.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_as);            ///< Registry operation: create a new context using interface of another context.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_from);          ///< Registry operation: create a new context using interface obtained from a slot of another context.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_params);        ///< Registry operation: create a parameters list context.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_ptr);           ///< Registry operation: create a pointer context.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_dptr_array);    ///< Registry operation: create a data pointer array context.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__invoke);               ///< Registry operation: invoke context call.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__unassign);             ///< Registry operation: unassign a context slot.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign);               ///< Registry operation: assign a value to a context slot.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign_slot);          ///< Registry operation: assign context slot to another context slot.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign_slot_weak);     ///< Registry operation: assign context slot to another context slot (unsetting reference counter).
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign_call);          ///< Registry operation: set context slot to result of another context call.
ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign_call_weak);     ///< Registry operation: set context slot to result of another context call (unsetting reference counter).

#endif // _ARCHI_APP_REGISTRY_FUN_H_

