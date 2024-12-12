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
 * @brief Macros for linked lists.
 */

#pragma once
#ifndef _ARCHI_UTIL_LIST_DEF_H_
#define _ARCHI_UTIL_LIST_DEF_H_

#include "archi/util/list.typ.h"

/**
 * @brief Declare/define linked list link function.
 */
#define ARCHI_LIST_LINK_FUNC(name) archi_status_t name( \
        const archi_list_node_t *prev, /* Previous node. */ \
        const archi_list_node_t *next, /* Next node. */ \
        void *data) // Function data.

/**
 * @brief Declare/define linked list node function.
 */
#define ARCHI_LIST_NODE_FUNC(name) archi_status_t name( \
        const archi_list_node_t *node, /* Current node. */ \
        bool is_head, /* Whether the current node is the list head. */ \
        bool is_tail, /* Whether the current node is the list tail. */ \
        void *data) // Function data.

/**
 * @brief Declare/define linked list action function.
 */
#define ARCHI_LIST_ACT_FUNC(name) archi_status_t name( \
        archi_list_node_t *node, /* Current node. */ \
        void *data) // Function data.

#endif // _ARCHI_UTIL_LIST_DEF_H_

