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
 * @brief Reference counter API.
 */

#pragma once
#ifndef _ARCHIPELAGO_BASE_REF_COUNT_FUN_H_
#define _ARCHIPELAGO_BASE_REF_COUNT_FUN_H_

#include "archipelago/base/ref_count.typ.h"

#include <stdbool.h>

/**
 * @brief Allocate and initialize a new reference counter object.
 *
 * The reference counter is initialized with the value of 1.
 *
 * @param destructor_fn  The destructor function to invoke when the reference count reaches zero.
 *                       Must not be NULL.
 * @param destructor_data Pointer to data that will be passed to the destructor function.
 *
 * @return Pointer to the newly allocated reference counter on success,
 *         or NULL if allocation fails or destructor_fn is NULL.
 *
 * @note
 * The returned pointer should only be manipulated through the provided
 * increment and decrement functions.
 * When the reference count reaches zero, the destructor will be called and
 * the object will be freed automatically, as well as the counter itself.
 */
archi_reference_count_t
archi_reference_count_alloc(
        archi_destructor_func_t destructor_fn,
        void *destructor_data
);

/**
 * @brief Deallocate a new reference counter object unconditionally.
 *
 * The associated destructor function is not called.
 *
 * @param ref_count Pointer to a valid reference counter object.
 *
 * @note
 * Passing NULL as ref_count is a no-op.
 */
void
archi_reference_count_free(
        archi_reference_count_t ref_count
);

/**
 * @brief Increment the reference count.
 *
 * Signals that an additional user holds a reference to the managed object.
 * This function is thread-safe and may be called from multiple threads.
 *
 * @param ref_count Pointer to a valid reference counter object.
 *
 * @note
 * Passing NULL as ref_count is a no-op.
 */
void
archi_reference_count_increment(
        archi_reference_count_t ref_count
);

/**
 * @brief Decrement the reference count.
 *
 * If the reference count drops to zero, the associated destructor function
 * is called with the provided data, and the reference counter object is freed.
 * This function is thread-safe and may be called from multiple threads.
 *
 * @param ref_count Pointer to a valid reference counter object.
 *
 * @return true if the destructor was called and the associated object was destroyed;
 *         false otherwise.
 *
 * @note
 * Passing NULL as ref_count is a no-op and returns false.
 *
 * @warning
 * After this function returns true, the memory pointed to by ref_count is invalid and
 * must not be accessed.
 */
bool
archi_reference_count_decrement(
        archi_reference_count_t ref_count
);

#endif // _ARCHIPELAGO_BASE_REF_COUNT_FUN_H_

