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
#ifndef _ARCHI_BASE_POINTER_FUN_H_
#define _ARCHI_BASE_POINTER_FUN_H_

#include "archi_base/pointer.typ.h"
#include "archi_base/error.typ.h"

#include <stdbool.h>
#include <stdalign.h>


/*****************************************************************************/
// Pointer attributes interface
/*****************************************************************************/

/**
 * @brief Compute pointer attributes (all bits except memory type) for a primitive data type.
 *
 * Data size (length * stride) must be not greater than ARCHI_POINTER_DATA_SIZE_MAX.
 * Stride must be non-zero. Alignment requirement must be a power of two.
 * Stride must be divisible by alignment requirement.
 *
 * @return Primitive data pointer attributes, or (archi_pointer_attr_t)-1 on error.
 */
archi_pointer_attr_t
archi_pointer_attr__pdata(
        size_t length,    ///< [in] Data length.
        size_t stride,    ///< [in] Data stride.
        size_t alignment, ///< [in] Data alignment.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Compute data pointer attributes for primitive data of specified length and type.
 */
#define ARCHI_POINTER_ATTR__PDATA(length, type) \
    archi_pointer_attr__pdata((length), sizeof(type), alignof(type), NULL)

/**
 * @brief Compute pointer attributes (all bits except pointer type) for a complex data type.
 *
 * Data type tag must not be greater than ARCHI_POINTER_DATA_TAG_MAX.
 *
 * @return Complex data pointer attributes, or (archi_pointer_attr_t)-1 on error.
 */
archi_pointer_attr_t
archi_pointer_attr__cdata(
        archi_pointer_attr_t tag ///< [in] Data type tag.
);

/**
 * @brief Compute function pointer attributes.
 *
 * Function type tag must not be greater than ARCHI_POINTER_FUNC_TAG_MAX.
 *
 * @return Function pointer attributes, or 0 on error.
 */
archi_pointer_attr_t
archi_pointer_attr__func(
        archi_pointer_attr_t tag ///< [in] Function type tag.
);

/**
 * @brief Unpack pointer attributes.
 *
 * This function never fails, because there are no forbidden attributes values.
 */
void
archi_pointer_attr_unpk(
        archi_pointer_attr_t attr, ///< [in] Pointer attributes.
        unsigned char *out_kind, ///< [out] Kind of the pointee.
        archi_pointer_attr_t *out_tag, ///< [out] Type tag.
        size_t *out_length,   ///< [out] Data length.
        size_t *out_stride,   ///< [out] Data stride.
        size_t *out_alignment ///< [out] Data alignment.
);

/**
 * @brief Unpack attributes of pointer to primitive data.
 *
 * @return True if attributes describes primitive data, false otherwise.
 */
bool
archi_pointer_attr_unpk__pdata(
        archi_pointer_attr_t attr, ///< [in] Pointer attributes.
        size_t *out_length,    ///< [out] Data length.
        size_t *out_stride,    ///< [out] Data stride.
        size_t *out_alignment, ///< [out] Data alignment.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Unpack attributes of pointer to complex data.
 *
 * @return True if attributes describes complex data, false otherwise.
 */
bool
archi_pointer_attr_unpk__cdata(
        archi_pointer_attr_t attr, ///< [in] Pointer attributes.
        archi_pointer_attr_t *out_tag, ///< [out] Data type tag.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Unpack attributes of pointer to a function.
 *
 * @return True if attributes describes function, false otherwise.
 */
bool
archi_pointer_attr_unpk__func(
        archi_pointer_attr_t attr, ///< [in] Pointer attributes.
        archi_pointer_attr_t *out_tag, ///< [out] Function type tag.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Check compatibility of pointer attributes with sample attributes.
 *
 * The operation is asymmetric. Attributes `attr` are considered compatible with `sample`,
 * if `sample` can be used in place of `attr` without causing undefined behavior.
 *
 * Data and function attributes are never compatible.
 * Function attributes with tag #0 is compatible with any function attributes.
 * Data attributes with tag #0 is compatible with any data attributes (both primitive and complex).
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
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
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
archi_rcpointer(
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
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
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
        archi_rcpointer_t entity, ///< [in] Owned entity.
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
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
        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_BASE_POINTER_FUN_H_

