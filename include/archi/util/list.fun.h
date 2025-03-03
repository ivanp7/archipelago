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
 * @brief Linked list operations.
 */

#pragma once
#ifndef _ARCHI_UTIL_LIST_FUN_H_
#define _ARCHI_UTIL_LIST_FUN_H_

#include "archi/util/list.typ.h"
#include "archi/util/container.typ.h"

/**
 * @brief Insert sublist to a linked list.
 *
 * @warning Sublist must be an independent list.
 *
 * If function is null, sublist is inserted immediately at the start.
 *
 * @return Positive value if there was no insertion,
 * zero if insertion took place, negative value on error.
 */
archi_status_t
archi_list_insert_sublist(
        archi_list_t *list, ///< [in,out] Linked list.
        archi_list_t *sublist, ///< [in,out] Inserted sublist.

        archi_list_link_func_t where_fn, ///< [in] Insertion point specifier.
        void *where_fn_data, ///< [in,out] Insertion point specifier data.

        bool start_from_head ///< [in] Whether to start traversing from the head.
);

/**
 * @brief Insert node to a linked list.
 *
 * @warning Inserted node must be an independent node.
 *
 * If function is null, node is inserted immediately at the start.
 *
 * @return Positive value if there was no insertion,
 * zero if insertion took place, negative value on error.
 */
archi_status_t
archi_list_insert_node(
        archi_list_t *list, ///< [in,out] Linked list.
        archi_list_node_t *node, ///< [in,out] Inserted node.

        archi_list_link_func_t where_fn, ///< [in] Insertion point specifier.
        void *where_fn_data, ///< [in,out] Insertion point specifier data.

        bool start_from_head ///< [in] Whether to start traversing from the head.
);

/**
 * @brief Cut sublist from a linked list.
 *
 * @warning Sublist must be fully contained within the list.
 *
 * @return True if the cut took place, otherwise false.
 */
bool
archi_list_cut_sublist(
        archi_list_t *restrict list,   ///< [in,out] Linked list.
        archi_list_t *restrict sublist ///< [in,out] Sublist to cut out.
);

/**
 * @brief Cut node from a linked list.
 *
 * @warning Node must belong to the list.
 *
 * @return True if the cut took place, otherwise false.
 */
bool
archi_list_cut_node(
        archi_list_t *list,     ///< [in,out] Linked list.
        archi_list_node_t *node ///< [in,out] Node to cut out.
);

/**
 * @brief Remove nodes from a linked list.
 *
 * If the node specifier function is null, all nodes are removed.
 *
 * @return Positive value if only a list part was traversed,
 * zero if the whole list was traversed, negative value on error.
 */
archi_status_t
archi_list_remove_nodes(
        archi_list_t *list, ///< [in,out] Linked list.

        archi_list_node_func_t which_fn, ///< [in] Removed nodes specifier.
        void *which_fn_data, ///< [in,out] Removed nodes specifier data.

        archi_list_act_func_t free_fn, ///< [in] Node memory deallocator.
        void *free_fn_data, ///< [in, out] Node memory deallocator data.

        bool start_from_head, ///< [in] Whether to start traversing from the head.
        size_t limit, ///< [in] Maximum number of nodes to remove if non-zero.

        size_t *num_removed ///< [out] Number of removed nodes.
);

/**
 * @brief Traverse a linked list.
 *
 * If the node specifier function is null, all nodes are processed.
 *
 * @return Positive value if only a list part was traversed,
 * zero if the whole list was traversed, negative value on error.
 */
archi_status_t
archi_list_traverse(
        archi_list_t *list, ///< [in,out] Linked list.

        archi_list_node_func_t which_fn, ///< [in] Processed nodes specifier.
        void *which_fn_data, ///< [in,out] Processed nodes specifier data.

        archi_list_act_func_t act_fn, ///< [in] Node memory action function.
        void *act_fn_data, ///< [in, out] Node memory action function data.

        bool start_from_head, ///< [in] Whether to start traversing from the head.
        size_t limit, ///< [in] Maximum number of nodes to process if non-zero.

        size_t *num_counted ///< [out] Number of counted nodes.
);

/*****************************************************************************/

/**
 * @brief Copy a string for a node name.
 *
 * @return Node name.
 */
char*
archi_list_node_copy_name(
        const char *name ///< [in] Copied string.
);

/*****************************************************************************/

/**
 * @brief Linked list link function -- select every nth link.
 *
 * @return 0 if the current link matches, otherwise 1.
 */
ARCHI_LIST_LINK_FUNC(archi_list_link_func_select_every_nth);
/**
 * @brief Linked list node function -- select every nth node.
 *
 * @return 0 if the current node matches, otherwise 1.
 */
ARCHI_LIST_NODE_FUNC(archi_list_node_func_select_every_nth);

/**
 * @brief Linked list node function -- select named nodes with the specified name.
 *
 * @return 0 if the current node matches, otherwise 1.
 */
ARCHI_LIST_NODE_FUNC(archi_list_node_func_select_by_name);

/**
 * @brief Linked list action function -- copy node pointer.
 */
ARCHI_LIST_ACT_FUNC(archi_list_act_func_extract_node);

/**
 * @brief Linked list action function -- free base node memory.
 */
ARCHI_LIST_ACT_FUNC(archi_list_act_func_free);

/**
 * @brief Linked list action function -- free named node memory.
 */
ARCHI_LIST_ACT_FUNC(archi_list_act_func_free_named);

/*****************************************************************************/

/**
 * @brief Linked list: container element insertion function.
 */
ARCHI_CONTAINER_INSERT_FUNC(archi_list_container_insert);

/**
 * @brief Linked list: container element removal function.
 */
ARCHI_CONTAINER_REMOVE_FUNC(archi_list_container_remove);

/**
 * @brief Linked list: container element extraction function.
 */
ARCHI_CONTAINER_EXTRACT_FUNC(archi_list_container_extract);

/**
 * @brief Linked list: container element traversal function.
 */
ARCHI_CONTAINER_TRAVERSE_FUNC(archi_list_container_traverse);

/**
 * @brief Linked list container interface.
 */
extern
const archi_container_interface_t archi_list_container_interface;

#endif // _ARCHI_UTIL_LIST_FUN_H_

