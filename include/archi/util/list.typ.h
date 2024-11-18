/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Linked list types.
 */

#pragma once
#ifndef _ARCHI_UTIL_LIST_TYP_H_
#define _ARCHI_UTIL_LIST_TYP_H_

#include "archi/util/status.typ.h"

#include <stdbool.h>
#include <stddef.h>

struct archi_list_node;

/**
 * @brief Linked list node base.
 */
typedef struct archi_list_node {
    struct archi_list_node *prev; ///< Previous node.
    struct archi_list_node *next; ///< Next node.
} archi_list_node_t;

/**
 * @brief Linked list.
 */
typedef struct archi_list {
    archi_list_node_t *head; ///< Linked list head.
    archi_list_node_t *tail; ///< Linked list tail.
} archi_list_t;

/*****************************************************************************/

/**
 * @brief Sublist node.
 */
typedef struct archi_list_node_sublist {
    archi_list_node_t link; ///< Node links.
    archi_list_t sublist; ///< Sublist.
} archi_list_node_sublist_t;

/**
 * @brief Named node.
 */
typedef struct archi_list_node_named {
    archi_list_node_t link; ///< Node links.
    char *name; ///< Node name.
} archi_list_node_named_t;

/**
 * @brief Value type.
 */
typedef enum archi_value_type {
    ARCHI_VALUE_NULL = 0, ///< No value.

    ARCHI_VALUE_UINT8,      ///< 8-bit unsigned integer.
    ARCHI_VALUE_UINT16,     ///< 16-bit unsigned integer.
    ARCHI_VALUE_UINT32,     ///< 32-bit unsigned integer.
    ARCHI_VALUE_UINT64,     ///< 64-bit unsigned integer.

    ARCHI_VALUE_SINT8,      ///< 8-bit signed integer.
    ARCHI_VALUE_SINT16,     ///< 16-bit signed integer.
    ARCHI_VALUE_SINT32,     ///< 32-bit signed integer.
    ARCHI_VALUE_SINT64,     ///< 64-bit signed integer.

    ARCHI_VALUE_FLOAT16,    ///< 16-bit floating-point number.
    ARCHI_VALUE_FLOAT32,    ///< 32-bit floating-point number.
    ARCHI_VALUE_FLOAT64,    ///< 64-bit floating-point number.

    ARCHI_VALUE_POINTER,    ///< Pointer to data or a function.

    ARCHI_VALUE_STRING,     ///< Null-terminated string.
    ARCHI_VALUE_DATA,       ///< Binary data.

    ARCHI_VALUE_NESTED,     ///< Nested node.
    ARCHI_VALUE_LIST,       ///< Nested list.
} archi_value_type_t;

/**
 * @brief Value handle.
 *
 * Minimum size of memory pointed to by ptr is (size * num_of).
 */
typedef struct archi_value {
    void *ptr; ///< Pointer to the first value element.

    size_t size; ///< Size of a value element, or zero if unknown.
    size_t num_of; ///< Number of value elements.

    archi_value_type_t type; ///< Value element type.
} archi_value_t; ///< Node value(s).

/**
 * @brief Value list node.
 */
typedef struct archi_list_node_value {
    archi_list_node_named_t base; ///< Named node base.
    archi_value_t value; ///< Node value.
} archi_list_node_value_t;

/*****************************************************************************/

/**
 * @brief Linked list link function.
 *
 * @return Positive value if the test failed, zero if the test passed,
 * negative value on traversal stop request.
 */
typedef archi_status_t (*archi_list_link_func_t)(
        const archi_list_node_t *next, ///< [in] Next node.
        const archi_list_node_t *prev, ///< [in] Previous node.
        void *data ///< [in,out] Function data.
);

/**
 * @brief Linked list node function.
 *
 * @return Positive value if the test failed, zero if the test passed,
 * negative value on traversal stop request.
 */
typedef archi_status_t (*archi_list_node_func_t)(
        const archi_list_node_t *node, ///< [in] Current node.
        bool is_head, ///< [in] Whether the current node is the list head.
        bool is_tail, ///< [in] Whether the current node is the list tail.
        void *data ///< [in,out] Function data.
);

/**
 * @brief Linked list action function.
 *
 * @return Zero on success, non-zero value on error.
 */
typedef archi_status_t (*archi_list_act_func_t)(
        archi_list_node_t *node, ///< [in] Current node.
        void *data ///< [in,out] Function data.
);

#endif // _ARCHI_UTIL_LIST_TYP_H_

