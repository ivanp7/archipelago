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
 * @brief Operations with aggregate objects.
 */

#pragma once
#ifndef _ARCHI_AGGR_API_INTERFACE_FUN_H_
#define _ARCHI_AGGR_API_INTERFACE_FUN_H_

#include "archi/aggr/api/handle.typ.h"
#include "archi/aggr/api/interface.typ.h"

#include <stdbool.h>


/**
 * @brief Get aggregate type interface.
 *
 * @return Pointer to the interface of the aggregate object.
 */
archi_rcpointer_t
archi_aggr_interface(
        archi_aggr_t aggregate ///< [in] Aggregate object handle.
);

/**
 * @brief Get aggregate type metadata.
 *
 * @return Pointer to type metadata.
 */
archi_rcpointer_t
archi_aggr_metadata(
        archi_aggr_t aggregate ///< [in] Aggregate object handle.
);

/**
 * @brief Get aggregate object pointer.
 *
 * @return Pointer to the object.
 */
archi_rcpointer_t
archi_aggr_object(
        archi_aggr_t aggregate ///< [in] Aggregate object handle.
);

/**
 * @brief Get aggregate type layout.
 *
 * @return Aggregate type layout.
 */
archi_layout_struct_t
archi_aggr_layout(
        archi_aggr_t aggregate ///< [in] Aggregate object handle.
);

/**
 * @brief Get length of flexible array of aggregate object.
 *
 * @return Flexible array length.
 */
size_t
archi_aggr_fam_length(
        archi_aggr_t aggregate ///< [in] Aggregate object handle.
);

/**
 * @brief Get full size of aggregate object including flexible array member.
 *
 * @return Full size of an object in bytes.
 */
size_t
archi_aggr_full_size(
        archi_aggr_t aggregate ///< [in] Aggregate object handle.
);

/**
 * @brief Get number of references in aggregate object.
 *
 * @return Number of references.
 */
size_t
archi_aggr_num_references(
        archi_aggr_t aggregate ///< [in] Aggregate object handle.
);

/**
 * @brief Get reference #index in aggregate object.
 *
 * @return Reference #index, or NULL in case of error.
 */
archi_rcpointer_t
archi_aggr_reference(
        archi_aggr_t aggregate, ///< [in] Aggregate object handle.
        size_t index ///< [in] Index of a reference.
);

/*****************************************************************************/

/**
 * @brief Allocate an aggregate object.
 *
 * For aggregate types without a flexible array member,
 * `flexarr_length` must be zero.
 *
 * @return Aggregate object handle.
 */
archi_aggr_t
archi_aggr_allocate(
        archi_rcpointer_t interface, ///< [in] Aggregate type interface.
        archi_rcpointer_t metadata, ///< [in] Aggregate type metadata.

        size_t fam_length, ///< [in] Flexible array length.

        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Deallocate an aggregate object.
 */
void
archi_aggr_free(
        archi_aggr_t aggregate ///< [in] Aggregate object handle.
);

/**
 * @brief Get aggregate object member.
 *
 * If `submember_depth` is zero, the object itself is returned.
 *
 * When `immediate_ref` is true, pointer to the referenced object is returned,
 * instead of the pointer to the pointer member.
 *
 * @return Pointer to object member, or NULL in case of error.
 */
archi_rcpointer_t
archi_aggr_get(
        archi_aggr_t aggregate, ///< [in] Aggregate object handle.

        const archi_aggr_member_spec_t submember[], ///< [in] Submember specifier array.
        size_t submember_depth, ///< [in] Length of member specifier array.

        bool immediate_ref, ///< [in] Immediate reference mode.

        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

/**
 * @brief Set aggregate object member.
 *
 * `submember_depth` must not be zero.
 */
void
archi_aggr_set(
        archi_aggr_t aggregate, ///< [in] Aggregate object handle.

        const archi_aggr_member_spec_t submember[], ///< [in] Submember specifier array.
        size_t submember_depth, ///< [in] Length of member specifier array.

        archi_rcpointer_t value, ///< [in] Value to set.

        ARCHI_ERROR_PARAM_DECL ///< [out] Error.
);

#endif // _ARCHI_AGGR_API_INTERFACE_FUN_H_

