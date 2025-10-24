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
 * @brief The application context interface type.
 */

#pragma once
#ifndef _ARCHI_CONTEXT_API_INTERFACE_TYP_H_
#define _ARCHI_CONTEXT_API_INTERFACE_TYP_H_

#include "archi/context/api/slot.typ.h"
#include "archipelago/base/pointer.typ.h"
#include "archipelago/base/parameters.typ.h"
#include "archipelago/base/status.typ.h"

/**
 * @brief Declare/define context initialization function.
 *
 * This function is intended to initialize/allocate resources (contexts).
 *
 * @return Status code.
 */
#define ARCHI_CONTEXT_INIT_FUNC(func_name) archi_status_t func_name( \
        archi_pointer_t **context, /* [out] Context data. */ \
        const archi_parameter_list_t *params) /* [in] Initialization parameters. */

/**
 * @brief Context initialization function type.
 */
typedef ARCHI_CONTEXT_INIT_FUNC((*archi_context_init_func_t));

/**
 * @brief Declare/define context finalization function.
 *
 * This function is intended to finalize/release resources (contexts).
 */
#define ARCHI_CONTEXT_FINAL_FUNC(func_name) void func_name( \
        archi_pointer_t *context) /* [in] Context data. */

/**
 * @brief Context finalization function type.
 */
typedef ARCHI_CONTEXT_FINAL_FUNC((*archi_context_final_func_t));

/**
 * @brief Declare/define context slot getter function.
 *
 * This function is intended to retrieve resources from a context.
 *
 * @return Status code.
 */
#define ARCHI_CONTEXT_GET_FUNC(func_name) archi_status_t func_name( \
        archi_pointer_t *context, /* [in,out] Context data. */ \
        const archi_context_slot_t slot, /* [in] Slot designator. */ \
        archi_pointer_t *value) /* [out] Place to store the gotten value. */

/**
 * @brief Context slot getter function type.
 */
typedef ARCHI_CONTEXT_GET_FUNC((*archi_context_get_func_t));

/**
 * @brief Declare/define context slot setter function.
 *
 * This function is intended to provide resources to a context.
 *
 * @return Status code.
 */
#define ARCHI_CONTEXT_SET_FUNC(func_name) archi_status_t func_name( \
        archi_pointer_t *context, /* [in,out] Context data. */ \
        const archi_context_slot_t slot, /* [in] Slot designator. */ \
        const archi_pointer_t value) /* [in] Value to set. */

/**
 * @brief Context slot setter function type.
 */
typedef ARCHI_CONTEXT_SET_FUNC((*archi_context_set_func_t));

/**
 * @brief Declare/define context action function.
 *
 * This function is intended to perform actions within contexts.
 *
 * @return Status code.
 */
#define ARCHI_CONTEXT_ACT_FUNC(func_name) archi_status_t func_name( \
        archi_pointer_t *context, /* [in,out] Context data. */ \
        const archi_context_slot_t action, /* [in] Action designator. */ \
        const archi_parameter_list_t *params) /* [in] Action parameters. */

/**
 * @brief Context action function type.
 */
typedef ARCHI_CONTEXT_ACT_FUNC((*archi_context_act_func_t));

/*****************************************************************************/

/**
 * @brief Context interface functions.
 */
typedef struct archi_context_interface {
    archi_context_init_func_t init_fn;   ///< Context initialization function.
    archi_context_final_func_t final_fn; ///< Context finalization function.

    archi_context_get_func_t get_fn; ///< Context slot getter function.
    archi_context_set_func_t set_fn; ///< Context slot setter function.

    archi_context_act_func_t act_fn; ///< Context action function.
} archi_context_interface_t;

#endif // _ARCHI_CONTEXT_API_INTERFACE_TYP_H_

