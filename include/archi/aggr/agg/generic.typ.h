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
 * @brief Meta description of aggregate types.
 */

#pragma once
#ifndef _ARCHI_AGGR_AGG_GENERIC_TYP_H_
#define _ARCHI_AGGR_AGG_GENERIC_TYP_H_

#include "archi_base/pointer.typ.h"
#include "archi_base/pointer.def.h"
#include "archi_base/error.typ.h"
#include "archi_base/util/size.typ.h"
#include "archi_base/util/size.def.h"

#include <stddef.h> // for offsetof()


struct archi_aggr_member;

///////////////////////////////////////////////////////////////////////
// Description of a value type (of aggregate member)
///////////////////////////////////////////////////////////////////////

/**
 * @brief Description of a value type of an aggregate member.
 *
 * If `tag` is zero, data type is considered primitive.
 * Otherwise, data type is complex with the specified tag.
 */
typedef struct archi_aggr_member_type__value {
    archi_layout_type_t layout; ///< Value member type layout.

    archi_pointer_attr_t value_tag; ///< Data type tag of the member type.
} archi_aggr_member_type__value_t;

/**
 * @brief Description literal of a value type.
 */
#define ARCHI_AGGR_MEMBER_TYPE__VALUE(value_type, value_type_tag)   \
    {.layout = ARCHI_LAYOUT_TYPE(value_type),                       \
        .value_tag = (value_type_tag)}

///////////////////////////////////////////////////////////////////////
// Description of a pointer type (of aggregate member)
///////////////////////////////////////////////////////////////////////

/**
 * @brief Description of a pointer type of an aggregate member.
 *
 * `pointee_tag` is used when `pointee_kind` is `ARCHI_POINTEE__CDATA` or `ARCHI_POINTEE__FUNC`.
 * `pointee_layout` is used when `pointee_kind` is `ARCHI_POINTEE__PDATA`.
 */
typedef struct archi_aggr_member_type__pointer {
    archi_layout_type_t layout; ///< Pointer member type layout.

    unsigned char pointee_kind; ///< Pointee kind.
    union {
        archi_pointer_attr_t pointee_tag; ///< Data/function type tag for the pointee type.
        archi_layout_array_t pointee_layout; ///< Array layout of the pointee primitive data type.
    };
} archi_aggr_member_type__pointer_t;

/**
 * @brief Description literal of a pointer type of an aggregate member (arbitrary pointee).
 */
#define ARCHI_AGGR_MEMBER_TYPE__POINTER(ptr_type)   \
    {.layout = ARCHI_LAYOUT_TYPE(ptr_type),         \
        .pointee_kind = ARCHI_POINTEE__UNSPEC}

/**
 * @brief Description literal of a pointer type of an aggregate member (pointee is primitive data).
 */
#define ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_PDATA(ptr_type, poe_type, num_poe)   \
    {.layout = ARCHI_LAYOUT_TYPE(ptr_type),                                     \
        .pointee_kind = ARCHI_POINTEE__PDATA,                                   \
        .pointee_layout = ARCHI_LAYOUT_ARRAY(poe_type, (num_poe))}

/**
 * @brief Description literal of a pointer type of an aggregate member (pointee is complex data).
 */
#define ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_CDATA(ptr_type, poe_tag) \
    {.layout = ARCHI_LAYOUT_TYPE(ptr_type),                         \
        .pointee_kind = ARCHI_POINTEE__CDATA,                       \
        .pointee_tag = (poe_tag)}

/**
 * @brief Description literal of a pointer type of an aggregate member (pointee is function).
 */
#define ARCHI_AGGR_MEMBER_TYPE__POINTER_TO_FUNC(ptr_type, poe_tag)  \
    {.layout = ARCHI_LAYOUT_TYPE(ptr_type),                         \
        .pointee_kind = ARCHI_POINTEE__FUNC,                        \
        .pointee_tag = (poe_tag)}

///////////////////////////////////////////////////////////////////////
// Description of an aggregate type (of aggregate member)
///////////////////////////////////////////////////////////////////////

/**
 * @brief Description of an aggregate member type.
 */
typedef struct archi_aggr_member_type__aggregate {
    archi_layout_type_t layout; ///< Aggregate member type layout.

    size_t num_members; ///< Total number of submembers.
    const struct archi_aggr_member *members; ///< Array of submember descriptions.
} archi_aggr_member_type__aggregate_t;

/**
 * @brief Description literal of an aggregate type.
 *
 * `member_array` must be an array variable.
 */
#define ARCHI_AGGR_MEMBER_TYPE__AGGREGATE(aggr_type, member_array)  \
    {.layout = ARCHI_LAYOUT_TYPE(aggr_type),                        \
        .num_members = ARCHI_LENGTH_ARRAY(member_array), .members = (member_array)}

///////////////////////////////////////////////////////////////////////
// Description of an aggregate member
///////////////////////////////////////////////////////////////////////

/**
 * @brief Kind of member type.
 */
typedef enum archi_aggr_member_kind {
    ARCHI_AGGR_MEMBER_KIND__VALUE = 0, ///< Value type (scalar member).
    ARCHI_AGGR_MEMBER_KIND__POINTER,   ///< Pointer type (scalar member).
    ARCHI_AGGR_MEMBER_KIND__AGGREGATE, ///< Aggregate type (aggregate member).
} archi_aggr_member_kind_t;

/**
 * @brief Description of an aggregate member.
 *
 * `name` must not be a non-empty member name string.
 * `offset` must be equal to `offsetof(aggr_type, member)`.
 * `length` must be:
 * - equal to 1 for ordinary (non-array) members;
 * - non-zero for array members;
 * - zero for the flexible array member.
 *
 * `layout` is a union member that is always valid (regardless of member type).
 */
typedef struct archi_aggr_member {
    const char *name; ///< Member name.
    size_t offset; ///< Member offset in bytes.

    size_t length; ///< Number of member (array) elements.

    archi_aggr_member_kind_t kind; ///< Kind of member type.
    union {
        const archi_layout_type_t *layout; ///< Member type layout.

        const archi_aggr_member_type__value_t *value;          ///< Value type description.
        const archi_aggr_member_type__pointer_t *pointer;      ///< Pointer type description.
        const archi_aggr_member_type__aggregate_t *aggregate;  ///< Aggregate type description.
    };
} archi_aggr_member_t;

/**
 * @brief Description literal of an aggregate member (value type).
 */
#define ARCHI_AGGR_MEMBER__VALUE(aggr_type, member, num_of, type_descr)             \
    {.name = #member, .offset = offsetof(aggr_type, member), .length = (num_of),    \
        .kind = ARCHI_AGGR_MEMBER_KIND__VALUE, .value = &(type_descr)}

/**
 * @brief Description literal of an aggregate member (pointer type).
 */
#define ARCHI_AGGR_MEMBER__POINTER(aggr_type, member, num_of, type_descr)           \
    {.name = #member, .offset = offsetof(aggr_type, member), .length = (num_of),    \
        .kind = ARCHI_AGGR_MEMBER_KIND__POINTER, .pointer = &(type_descr)}

/**
 * @brief Description literal of an aggregate member (aggregate type).
 */
#define ARCHI_AGGR_MEMBER__AGGREGATE(aggr_type, member, num_of, type_descr)         \
    {.name = #member, .offset = offsetof(aggr_type, member), .length = (num_of),    \
        .kind = ARCHI_AGGR_MEMBER_KIND__AGGREGATE, .aggregate = &(type_descr)}

///////////////////////////////////////////////////////////////////////
// Description of an aggregate type
///////////////////////////////////////////////////////////////////////

/**
 * @brief Description of an aggregate type.
 *
 * If length of the last member is zero, it is considered a flexible array member and is treated specially.
 *
 * `init_value` must not be NULL.
 * `init_value_fam` must not be NULL if the flexible array member exists.
 */
typedef struct archi_aggr_type {
    archi_aggr_member_type__aggregate_t top_level; ///< Description of top-level members.

    const void *init_value; ///< Object initialization value.
    const void *init_value_fam; ///< Flexible array element initialization value.
} archi_aggr_type_t;

/**
 * @brief Literal of an aggregate type description (without flexible array member).
 */
#define ARCHI_AGGR_TYPE(aggr_type, member_array)                                   \
    {.top_level = ARCHI_AGGR_MEMBER_TYPE__AGGREGATE(aggr_type, (member_array)),    \
        .init_value = &(aggr_type){0}}

/**
 * @brief Literal of an aggregate type description (with flexible array member).
 */
#define ARCHI_AGGR_TYPE_WITH_FAM(aggr_type, fam_elt_type, member_array)            \
    {.top_level = ARCHI_AGGR_MEMBER_TYPE__AGGREGATE(aggr_type, (member_array)),    \
        .init_value = &(aggr_type){0}, .init_value_fam = &(fam_elt_type){0}}

#endif // _ARCHI_AGGR_AGG_GENERIC_TYP_H_

