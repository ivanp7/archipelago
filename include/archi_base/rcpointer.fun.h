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
 * @brief Operations with reference-counted generic pointers.
 */

#pragma once
#ifndef _ARCHI_BASE_RCPOINTER_FUN_H_
#define _ARCHI_BASE_RCPOINTER_FUN_H_

#include "archi_base/rcpointer.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>


/**
 * @brief Attach a reference counter to a pointer.
 *
 * `ref_count` field is assigned only if the pointer
 * is non-null and is not referring to transient data.
 *
 * @return Pointer with a reference counter.
 */
archi_rcpointer_t
archi_rcpointer(
        archi_pointer_t p, ///< [in] Pointer.
        archi_refcount_t ref_count ///< [in] Reference counter.
);

/**
 * @brief Create a long-lived, writable copy of data on the heap.
 *
 * Pointer attributes (except type) are preserved.
 * Pointer type is set to ARCHI_POINTER_TYPE__DATA_WRITABLE.
 *
 * New reference counter is created for the copy.
 *
 * @return Pointer to the copied memory, or empty pointer in case of error.
 */
archi_rcpointer_t
archi_rcpointer_memcopy(
        archi_rcpointer_t source, ///< [in] Source memory.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Claim ownership of an entity.
 *
 * Reference counter of the entity gets incremented, unless it is transient memory.
 * In that case, a memory copy is allocated using archi_rcpointer_memcopy().
 * The resulting pointer is written to the output parameter.
 *
 * @return True on success, false on error.
 */
bool
archi_rcpointer_claim(
        archi_rcpointer_t *entity, ///< [in,out] Entity.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Abandon ownership of an entity.
 *
 * Reference counter of the entity gets decremented.
 */
void
archi_rcpointer_abandon(
        archi_rcpointer_t entity ///< [in] Entity.
);

/**
 * @brief Claim ownership of one entity and abandon ownership of another.
 *
 * If claiming ownership of the first entity failed, the second entity is not abandoned.
 *
 * @return True on success, false on error.
 */
bool
archi_rcpointer_replace(
        archi_rcpointer_t *claimed_entity, ///< [in,out] Claimed entity.
        archi_rcpointer_t abandoned_entity, ///< [in] Abandoned entity.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_BASE_RCPOINTER_FUN_H_

