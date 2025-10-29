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
 * @brief Meta description of C structures.
 */

#pragma once
#ifndef _ARCHIPELAGO_UTIL_STRUCT_TYP_H_
#define _ARCHIPELAGO_UTIL_STRUCT_TYP_H_

#include "archipelago/base/pointer.typ.h"

#include <stddef.h> // for size_t, offsetof()
#include <stdbool.h>

/**
 * @brief Type of a field.
 *
 * Value fields can have only TRANSP_DATA type.
 * Pointer fields can have any type.
 */
typedef enum archi_struct_field_type {
    ARCHI_STRUCT_FIELD_TYPE__DATA = 0,
    ARCHI_STRUCT_FIELD_TYPE__TRANSP_DATA,
    ARCHI_STRUCT_FIELD_TYPE__FUNCTION,
} archi_struct_field_type_t;

/**
 * @brief Description of a field.
 */
typedef struct archi_struct_field_info {
    const char *name; ///< Field name.
    size_t offset;    ///< Field offset in bytes.
    archi_struct_field_type_t type; ///< Field type.
    union {
        archi_pointer_attr_t tag; ///< Type tag.
        struct {
            size_t length;    ///< Number of data elements.
            size_t stride;    ///< Size of a data element in bytes.
            size_t alignment; ///< Alignment requirement of a data element in bytes.
        };
    };
} archi_struct_field_info_t;

/**
 * @brief Literal of a description of a value field.
 */
#define ARCHI_STRUCT_VFIELD_INFO(field, structure, len, str, align) \
    ARCHI_STRUCT_PFIELD_INFO__TRANSP_DATA(field, structure, (len), (str), (align))

/**
 * @brief Literal of a description of a pointer field (any data type).
 */
#define ARCHI_STRUCT_PFIELD_INFO__DATA(field, structure, type_tag)                                          \
    {.name = #field, .offset = offsetof(structure, field), .type = ARCHI_STRUCT_FIELD_TYPE__DATA,           \
    .tag = (type_tag)}

/**
 * @brief Literal of a description of a pointer field (transparent data type).
 */
#define ARCHI_STRUCT_PFIELD_INFO__TRANSP_DATA(field, structure, len, str, align)                            \
    {.name = #field, .offset = offsetof(structure, field), .type = ARCHI_STRUCT_FIELD_TYPE__TRANSP_DATA,    \
    .length = (len), .stride = (str), .alignment = (align)}

/**
 * @brief Literal of a description of a pointer field (function type).
 */
#define ARCHI_STRUCT_PFIELD_INFO__FUNCTION(field, structure, type_tag)                                      \
    {.name = #field, .offset = offsetof(structure, field), .type = ARCHI_STRUCT_FIELD_TYPE__FUNCTION,       \
    .tag = (type_tag)}

/**
 * @brief Description of a structure.
 */
typedef struct archi_struct_info {
    const archi_struct_field_info_t *vfield; ///< Array of value field descriptions.
    const archi_struct_field_info_t *pfield; ///< Array of pointer field descriptions.

    size_t num_vfields; ///< Number of value fields in the structure.
    size_t num_pfields; ///< Number of pointer fields in the structure.

    size_t size;      ///< Structure size in bytes.
    size_t alignment; ///< Structure alignment requirement.

    archi_rodata_t initializer; ///< Default structure object.
} archi_struct_info_t;

#endif // _ARCHIPELAGO_UTIL_STRUCT_TYP_H_

