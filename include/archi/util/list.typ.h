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
 * @brief Types for linked lists.
 */

#pragma once
#ifndef _ARCHI_UTIL_LIST_TYP_H_
#define _ARCHI_UTIL_LIST_TYP_H_

#include "archi/util/status.typ.h"
#include "archi/util/value.typ.h"

#include <stdbool.h>

struct archi_list_node;

/*****************************************************************************/

/**
 * @brief Declare/define linked list link function.
 *
 * @return Positive value if the test failed, zero if the test passed,
 * negative value on traversal stop request.
 */
#define ARCHI_LIST_LINK_FUNC(name) archi_status_t name( \
        const struct archi_list_node *prev, /* Previous node. */ \
        const struct archi_list_node *next, /* Next node. */ \
        size_t position, /* Current link position counter. */ \
        bool is_last, /* Whether the current link is the last. */ \
        void *data) /* Function data. */

/**
 * @brief Linked list link function.
 */
typedef ARCHI_LIST_LINK_FUNC((*archi_list_link_func_t));

/*****************************************************************************/

/**
 * @brief Declare/define linked list node function.
 *
 * @return Positive value if the test failed, zero if the test passed,
 * negative value on traversal stop request.
 */
#define ARCHI_LIST_NODE_FUNC(name) archi_status_t name( \
        const struct archi_list_node *node, /* Current node. */ \
        size_t position, /* Current node position counter. */ \
        bool is_last, /* Whether the current node is the last. */ \
        void *data) /* Function data. */

/**
 * @brief Linked list node function.
 */
typedef ARCHI_LIST_NODE_FUNC((*archi_list_node_func_t));

/*****************************************************************************/

/**
 * @brief Declare/define linked list action function.
 *
 * @return Zero on success, non-zero value on error.
 */
#define ARCHI_LIST_ACT_FUNC(name) archi_status_t name( \
        struct archi_list_node *node, /* Current node. */ \
        size_t position, /* Current node position counter. */ \
        void *data) /* Function data. */

/**
 * @brief Linked list action function.
 */
typedef ARCHI_LIST_ACT_FUNC((*archi_list_act_func_t));

/*****************************************************************************/

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
 * @brief Named node.
 */
typedef struct archi_list_node_named {
    archi_list_node_t link; ///< Node links.
    const char *name; ///< Node name.
} archi_list_node_named_t;

/**
 * @brief Named value list node.
 */
typedef struct archi_list_node_named_value {
    archi_list_node_named_t base; ///< Named node base.
    archi_value_t value; ///< Node value.
} archi_list_node_named_value_t;

/*****************************************************************************/

/**
 * @brief Linked list container.
 */
typedef struct archi_list_container_data {
    archi_list_t list; ///< Linked list.

    bool insert_to_head; ///< Whether insertion is done to the head.
    bool traverse_from_head; ///< Whether traversal is done from the head.
} archi_list_container_data_t;

#endif // _ARCHI_UTIL_LIST_TYP_H_

