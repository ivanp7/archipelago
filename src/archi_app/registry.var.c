/*****************************************************************************
    {C, C},
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
 * @brief Collection of supported context registry operations.
 */

#include "archi_app/registry.var.h"
#include "archi_app/registry.def.h"
#include "archi_app/registry.fun.h"


const archi_app_registry_operation_t
archi_app_registry_operations[] = {
    {.name = ARCHI_APP_REGISTRY_OP__DELETE, .function = archi_app_registry_op__delete},
    {.name = ARCHI_APP_REGISTRY_OP__ALIAS, .function = archi_app_registry_op__alias},
    {.name = ARCHI_APP_REGISTRY_OP__CREATE_AS, .function = archi_app_registry_op__create_as},
    {.name = ARCHI_APP_REGISTRY_OP__CREATE_FROM, .function = archi_app_registry_op__create_from},
    {.name = ARCHI_APP_REGISTRY_OP__CREATE_PARAMS, .function = archi_app_registry_op__create_params},
    {.name = ARCHI_APP_REGISTRY_OP__CREATE_PTR, .function = archi_app_registry_op__create_ptr},
    {.name = ARCHI_APP_REGISTRY_OP__CREATE_DPTR_ARRAY, .function = archi_app_registry_op__create_dptr_array},
    {.name = ARCHI_APP_REGISTRY_OP__INVOKE, .function = archi_app_registry_op__invoke},
    {.name = ARCHI_APP_REGISTRY_OP__UNASSIGN, .function = archi_app_registry_op__unassign},
    {.name = ARCHI_APP_REGISTRY_OP__ASSIGN, .function = archi_app_registry_op__assign},
    {.name = ARCHI_APP_REGISTRY_OP__ASSIGN_SLOT, .function = archi_app_registry_op__assign_slot},
    {.name = ARCHI_APP_REGISTRY_OP__ASSIGN_SLOT_WEAK, .function = archi_app_registry_op__assign_slot_weak},
    {.name = ARCHI_APP_REGISTRY_OP__ASSIGN_CALL, .function = archi_app_registry_op__assign_call},
    {.name = ARCHI_APP_REGISTRY_OP__ASSIGN_CALL_WEAK, .function = archi_app_registry_op__assign_call_weak},
    {0},
};

