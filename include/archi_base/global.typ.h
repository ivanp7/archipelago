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
 * @brief Types for global objects.
 */

#pragma once
#ifndef _ARCHI_BASE_GLOBAL_TYP_H_
#define _ARCHI_BASE_GLOBAL_TYP_H_

/**
 * @brief Declare/define global object getter function.
 *
 * This function is intended for obtaining pointers to global objects.
 */
#define ARCHI_GLOBAL_GET_FUNC(func_name)    void* func_name(void)

/**
 * @brief Global object getter function type.
 */
typedef ARCHI_GLOBAL_GET_FUNC((*archi_global_get_func_t));

/**
 * @brief Declare/define global object setter function.
 *
 * This function is intended for initializing global object pointers in loaded shared libraries.
 */
#define ARCHI_GLOBAL_SET_FUNC(func_name)    void func_name( \
        void *context) /* [in] Global object. */

/**
 * @brief Global object setter function type.
 *
 * Shared libraries loaded at runtime can use a symbol of this type
 * (typically obtained via dlsym() or a similar mechanism) to initialize
 * their own global object pointer with the pointer provided by the host application.
 */
typedef ARCHI_GLOBAL_SET_FUNC((*archi_global_set_func_t));

/**
 * @brief Global object initialization specification.
 */
typedef struct archi_global_init_spec {
    archi_global_get_func_t get_fn; ///< Global object getter function.
    const char *set_fn_symbol; ///< Symbol name of global object setter function.
} archi_global_init_spec_t;

#endif // _ARCHI_BASE_GLOBAL_TYP_H_

