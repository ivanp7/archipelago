/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Types for contexts.
 */

#pragma once
#ifndef _ARCHI_APP_CONTEXT_TYP_H_
#define _ARCHI_APP_CONTEXT_TYP_H_

#include "archi/util/container.typ.h"
#include "archi/util/status.typ.h"
#include "archi/util/value.typ.h"

/**
 * @brief Dummy context pointer used when the context initialization function is null.
 *
 * This pointer designates that there is no meaningful context pointer
 * to be used with a context interface. It is distinguished from the null pointer,
 * which signifies a failure of context initialization.
 * This pointer is not passed to context interface functions,
 * being replaced with the null pointer instead.
 */
#define ARCHI_CONTEXT_DUMMY ((void*)-1)

/**
 * @brief Declare/define context initialization function.
 *
 * This function is intended to initialize/allocate resources (contexts).
 *
 * @return Status code.
 */
#define ARCHI_CONTEXT_INIT_FUNC(func_name) archi_status_t func_name( \
        void **const restrict context, /* Place for pointer to the new context. */ \
        const archi_container_t config) /* Context configuration. */

/**
 * @brief Context initialization function.
 */
typedef ARCHI_CONTEXT_INIT_FUNC((*archi_context_init_func_t));

/**
 * @brief Declare/define context finalization function.
 *
 * This function is intended to finalize/release resources (contexts).
 */
#define ARCHI_CONTEXT_FINAL_FUNC(func_name) void func_name( \
        void *const context) /* Context to destroy. */

/**
 * @brief Context finalization function.
 */
typedef ARCHI_CONTEXT_FINAL_FUNC((*archi_context_final_func_t));

/**
 * @brief Declare/define context slot setter function.
 *
 * This function is intended to provide resources to a context.
 *
 * @return Status code.
 */
#define ARCHI_CONTEXT_SET_FUNC(func_name) archi_status_t func_name( \
        void *const restrict context, /* Context. */ \
        const char *const restrict slot, /* Slot. */ \
        const archi_value_t *const restrict value) /* Value to set. */

/**
 * @brief Context slot setter function.
 */
typedef ARCHI_CONTEXT_SET_FUNC((*archi_context_set_func_t));

/**
 * @brief Declare/define context slot getter function.
 *
 * This function is intended to retrieve resources from a context.
 *
 * @return Status code.
 */
#define ARCHI_CONTEXT_GET_FUNC(func_name) archi_status_t func_name( \
        void *const restrict context, /* Context. */ \
        const char *const restrict slot, /* Slot. */ \
        archi_value_t *const restrict value) /* Gotten value. */

/**
 * @brief Context slot getter function.
 */
typedef ARCHI_CONTEXT_GET_FUNC((*archi_context_get_func_t));

/**
 * @brief Declare/define context action function.
 *
 * This function is intended to perform actions within contexts.
 *
 * @return Status code.
 */
#define ARCHI_CONTEXT_ACT_FUNC(func_name) archi_status_t func_name( \
        void *const restrict context, /* Context. */ \
        const char *const restrict action, /* Action type. */ \
        const archi_container_t params) /* Action parameters. */

/**
 * @brief Context action function.
 */
typedef ARCHI_CONTEXT_ACT_FUNC((*archi_context_act_func_t));

/*****************************************************************************/

/**
 * @brief Context interface functions.
 */
typedef struct archi_context_interface {
    archi_context_init_func_t init_fn;   ///< Context initialization function.
    archi_context_final_func_t final_fn; ///< Context finalization function.

    archi_context_set_func_t set_fn; ///< Context slot setter function.
    archi_context_get_func_t get_fn; ///< Context slot getter function.

    archi_context_act_func_t act_fn; ///< Context action function.
} archi_context_interface_t;

/**
 * @brief A context.
 */
typedef struct archi_context {
    void *handle; ///< Context handle.
    const archi_context_interface_t *interface; ///< Context interface.
} archi_context_t;

#endif // _ARCHI_APP_CONTEXT_TYP_H_

