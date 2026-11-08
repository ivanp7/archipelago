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
 * @brief The manager description types.
 */

#pragma once
#ifndef _ARCHI_BASE_MANAGER_TYP_H_
#define _ARCHI_BASE_MANAGER_TYP_H_

/**
 * @brief Signature of a manager pointer getter function.
 *
 * This function is intended for obtaining the pointer to a manager.
 */
#define ARCHI_MANAGER_PTR_GET_FUNC(func_name)   void* func_name(void)

/**
 * @brief Manager pointer getter function type.
 */
typedef ARCHI_MANAGER_PTR_GET_FUNC((*archi_manager_ptr_get_func_t));

/**
 * @brief Signature of a manager pointer initialization function.
 *
 * This function is intended for initializing the pointer to a manager in loaded shared libraries.
 */
#define ARCHI_MANAGER_PTR_INIT_FUNC(func_name)  void func_name( \
        void *manager) /* [in] Pointer to manager. */

/**
 * @brief Manager setter function type.
 *
 * Shared libraries loaded at runtime can use a symbol of this type
 * (typically obtained via dlsym() or a similar mechanism) to initialize
 * their own manager pointer with the pointer provided by the host application.
 */
typedef ARCHI_MANAGER_PTR_INIT_FUNC((*archi_manager_ptr_init_func_t));

/**
 * @brief Manager initialization specification.
 */
typedef struct archi_manager_ptr_init_spec {
    const char *init_fn_symbol; ///< Symbol name of the manager pointer initialization function.
    archi_manager_ptr_get_func_t get_fn; ///< Manager pointer getter function.
} archi_manager_ptr_init_spec_t;

#endif // _ARCHI_BASE_MANAGER_TYP_H_

