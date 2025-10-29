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
 * @brief Operations with pointers.
 */

#pragma once
#ifndef _ARCHIPELAGO_BASE_POINTER_FUN_H_
#define _ARCHIPELAGO_BASE_POINTER_FUN_H_

#include "archipelago/base/pointer.typ.h"
#include "archipelago/base/error.typ.h"

#include <stdbool.h>
#include <stdalign.h>

/*****************************************************************************/
// Pointer attributes interface
/*****************************************************************************/

/**
 * @brief Compute pointer attributes (all bits except memory type) for a transparent data type.
 *
 * Data size (length * stride) must be not greater than ARCHI_POINTER_DATA_SIZE_MAX.
 * Stride must be non-zero. Alignment requirement must be a power of two.
 * Stride must be divisible by alignment requirement.
 *
 * @return Transparent data pointer attributes, or (archi_pointer_attr_t)-1 on error.
 */
archi_pointer_attr_t
archi_pointer_attr__transp_data(
        size_t length,    ///< [in] Data length.
        size_t stride,    ///< [in] Data stride.
        size_t alignment, ///< [in] Data alignment.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Compute data pointer attributes for data of given length and type.
 */
#define ARCHI_POINTER_ATTR__DATA_TYPE(length, type) \
    archi_pointer_attr__transp_data((length), sizeof(type), alignof(type), NULL)

/**
 * @brief Compute pointer attributes (all bits except pointer type) for an opaque data type.
 *
 * Data type tag must not be greater than ARCHI_POINTER_DATA_TAG_MAX.
 *
 * @return Opaque data pointer attributes, or (archi_pointer_attr_t)-1 on error.
 */
archi_pointer_attr_t
archi_pointer_attr__opaque_data(
        archi_pointer_attr_t tag ///< [in] Opaque data type tag.
);

/**
 * @brief Compute function pointer attributes.
 *
 * Function type tag must not be greater than ARCHI_POINTER_FUNCTION_TAG_MAX.
 *
 * @return Function pointer attributes, or 0 on error.
 */
archi_pointer_attr_t
archi_pointer_attr__function(
        archi_pointer_attr_t tag ///< [in] Function type tag.
);

/**
 * @brief Check if pointer attributes describes transparent data, and parse it.
 *
 * @return True if attributes describe transparent data, false otherwise.
 */
bool
archi_pointer_attr_parse__transp_data(
        archi_pointer_attr_t attr, ///< [in] Pointer attributes.
        size_t *out_length,    ///< [out] Data length.
        size_t *out_stride,    ///< [out] Data stride.
        size_t *out_alignment, ///< [out] Data alignment.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Check if pointer attributes describes opaque data, and parse it.
 *
 * @return True if attributes describe opaque data, false otherwise.
 */
bool
archi_pointer_attr_parse__opaque_data(
        archi_pointer_attr_t attr, ///< [in] Pointer attributes.
        archi_pointer_attr_t *out_tag, ///< [out] Opaque data type tag.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Check if pointer attributes describes function, and parse it.
 *
 * @return True if attributes describe function, false otherwise.
 */
bool
archi_pointer_attr_parse__function(
        archi_pointer_attr_t attr, ///< [in] Pointer attributes.
        archi_pointer_attr_t *out_tag, ///< [out] Function type tag.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Check compatibility of pointer attributes with sample attributes.
 *
 * The operation is asymmetric. Attributes `attr` are considered compatible with `sample`,
 * if `sample` can be used in place of `attr` without causing undefined behavior.
 *
 * Data and function attributes are never compatible.
 * Function type tag #0 is compatible with any other function type tag.
 * Opaque data type tag #0 is compatible with any other opaque type tag,
 * and any transparent type too.
 *
 * @return True if attributes are compatible, false otherwise.
 */
bool
archi_pointer_attr_compatible(
        archi_pointer_attr_t attr, ///< [in] Pointer attributes.
        archi_pointer_attr_t sample ///< [in] Sample attributes.
);

/*****************************************************************************/
// Pointer interface
/*****************************************************************************/

/**
 * @brief Check if pointer is valid.
 *
 * @return True if the pointer is valid, false otherwise.
 */
bool
archi_pointer_valid(
        archi_pointer_t p, ///< [in] Pointer.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Attach a reference counter to a pointer.
 *
 * `ref_count` field is assigned only if the pointer
 * is non-null and is not referring to stack.
 *
 * @return Pointer with a reference counter.
 */
archi_rcpointer_t
archi_pointer_with_refcount(
        archi_pointer_t p, ///< [in] Pointer.
        archi_reference_count_t ref_count ///< [in] Reference counter.
);

/*****************************************************************************/
// Reference counted pointer interface
/*****************************************************************************/

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
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Claim ownership of an entity.
 *
 * Reference counter of the entity gets incremented, unless it is data on stack.
 * In that case, a memory copy is allocated using archi_rcpointer_memcopy().
 *
 * @return Pointer to claimed entity, or null pointer with attributes == 0 in case of error.
 */
archi_rcpointer_t
archi_rcpointer_own(
        archi_rcpointer_t entity, ///< [in] Claimed entity.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

/**
 * @brief Abandon ownership of an entity.
 *
 * Reference counter of the entity gets decremented.
 */
void
archi_rcpointer_disown(
        archi_rcpointer_t entity ///< [in] Disowned entity.
);

/**
 * @brief Claim ownership of one entity and abandon ownership of another.
 *
 * If claiming ownership of the first entity failed, the second entity is not abandoned.
 *
 * @return Pointer to claimed entity, or null pointer with attributes == 0 in case of error.
 */
archi_rcpointer_t
archi_rcpointer_own_disown(
        archi_rcpointer_t owned_entity, ///< [in] Owned entity.
        archi_rcpointer_t disowned_entity, ///< [in] Disowned entity.
        ARCHI_ERROR_PARAMETER_DECL ///< [out] Error.
);

#endif // _ARCHIPELAGO_BASE_POINTER_FUN_H_

