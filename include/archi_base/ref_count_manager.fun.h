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
 * @brief The reference counting manager.
 */

#pragma once
#ifndef _ARCHI_BASE_REF_COUNT_MANAGER_FUN_H_
#define _ARCHI_BASE_REF_COUNT_MANAGER_FUN_H_

#include "archi_base/manager.typ.h"

#include <stdbool.h>


/**
 * @brief Retrieve the reference counting manager pointer.
 *
 * If the reference counting manager pointer hasn't been initialized yet, the function returns NULL.
 *
 * @return Pointer to the reference counting manager.
 */
ARCHI_MANAGER_PTR_GET_FUNC(archi_manager_ptr_get__ref_count);

/**
 * @brief Initialize the reference counting manager pointer.
 *
 * @pre
 *      manager != NULL
 *
 * @post
 *      Reference counting API is ready to be used.
 *
 * @note Subsequent invocations have no effect.
 *
 * @param[in] manager
 *      Reference counting manager.
 */
ARCHI_MANAGER_PTR_INIT_FUNC(archi_manager_ptr_init__ref_count);

/*****************************************************************************/

/**
 * @brief Obtain total number of times reference counters were incremented.
 *
 * This function counts archi_reference_count_alloc() and archi_reference_count_increment() calls.
 *
 * @return Total number of increments.
 */
unsigned long long
archi_reference_count_total_increments(void);

/**
 * @brief Obtain total number of times reference counters were decremented.
 *
 * This function counts archi_reference_count_decrement() calls.
 *
 * @return Total number of decrements.
 */
unsigned long long
archi_reference_count_total_decrements(void);

/**
 * @brief Check if negative reference count has been detected.
 *
 * @return False if all reference counters were non-negative, true otherwise.
 */
bool
archi_reference_count_negative_detected(void);

#endif // _ARCHI_BASE_REF_COUNT_MANAGER_FUN_H_

