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
 * @brief Linked list operations.
 */

#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h>
#include <string.h>

archi_status_t
archi_list_insert_sublist(
        archi_list_t *list,
        archi_list_t *sublist,

        archi_list_link_func_t where_fn,
        void *where_fn_data,

        bool start_from_head)
{
    if (list == NULL)
        return ARCHI_ERROR_MISUSE;

    if ((sublist == NULL) || (sublist->head == NULL) || (sublist->tail == NULL))
        return ARCHI_ERROR_MISUSE;

    if (list == sublist)
        return ARCHI_ERROR_MISUSE;

    archi_list_node_t *list_head = list->head;
    archi_list_node_t *list_tail = list->tail;

    if ((list_head == NULL) && (list_tail == NULL))
    {
        if ((where_fn == NULL) || (where_fn(NULL, NULL, where_fn_data) == 0))
        {
            *list = *sublist;
            return 0;
        }
        else
            return 1;
    }

    archi_list_node_t *node_prev, *node_next;
    if (start_from_head)
    {
        if (list_head == NULL)
            return ARCHI_ERROR_MISUSE;

        node_prev = list_head->prev;
        node_next = list_head;
    }
    else
    {
        if (list_tail == NULL)
            return ARCHI_ERROR_MISUSE;

        node_prev = list_tail;
        node_next = list_tail->next;
    }

    archi_status_t code;

    for (;;) // whole list
    {
        if (where_fn != NULL)
        {
            archi_list_node_t *prev = node_prev;
            archi_list_node_t *next = node_next;

            // Hide nodes beyond the list borders
            if ((list_head != NULL) && (node_next == list_head))
                prev = NULL;

            if ((list_tail != NULL) && (node_prev == list_tail))
                next = NULL;

            code = where_fn(prev, next, where_fn_data);
        }
        else
            code = 0;

        if (code < 0) // stop now
            break;
        else if (code == 0) // insert here
        {
            // Update sublist external links
            sublist->head->prev = node_prev;
            sublist->tail->next = node_next;

            // Update list internal links
            if (node_prev != NULL)
                node_prev->next = sublist->head;

            if (node_next != NULL)
                node_next->prev = sublist->tail;

            // Update list end pointers
            if ((list_head != NULL) && (node_next == list_head)) // regardless of node_prev value
                list->head = sublist->head;

            if ((list_tail != NULL) && (node_prev == list_tail)) // regardless of node_next value
                list->tail = sublist->tail;

            break;
        }
        // else proceed

        if (start_from_head)
        {
            if ((node_next == NULL) || ((list_tail != NULL) && (node_prev == list_tail)))
            {
                code = 1;
                break;
            }

            node_prev = node_next;
            node_next = node_next->next;
        }
        else
        {
            if ((node_prev == NULL) || ((list_head != NULL) && (node_next == list_head)))
            {
                code = 1;
                break;
            }

            node_next = node_prev;
            node_prev = node_prev->prev;
        }
    }

    return code;
}

archi_status_t
archi_list_insert_node(
        archi_list_t *list,
        archi_list_node_t *node,

        archi_list_link_func_t where_fn,
        void *where_fn_data,

        bool start_from_head)
{
    archi_list_t sublist = {.head = node, .tail = node};
    return archi_list_insert_sublist(list, &sublist, where_fn, where_fn_data, start_from_head);
}

bool
archi_list_cut_sublist(
        archi_list_t *restrict list,
        archi_list_t *restrict sublist)
{
    if ((sublist == NULL) || (sublist->head == NULL) || (sublist->tail == NULL))
        return false;

    if (list == sublist)
        return false;

    // Update list end pointers
    if (list != NULL)
    {
        bool with_head = (list->head == sublist->head);
        bool with_tail = (list->tail == sublist->tail);

        if (with_head && with_tail)
            list->head = list->tail = NULL;
        else if (with_head)
            list->head = sublist->tail->next;
        else if (with_tail)
            list->tail = sublist->head->prev;
    }

    // Update sublist external links
    if (sublist->head->prev != NULL)
        sublist->head->prev->next = sublist->tail->next;

    if (sublist->tail->next != NULL)
        sublist->tail->next->prev = sublist->head->prev;

    sublist->head->prev = NULL;
    sublist->tail->next = NULL;

    return true;
}

bool
archi_list_cut_node(
        archi_list_t *list,
        archi_list_node_t *node)
{
    archi_list_t sublist = {.head = node, .tail = node};
    return archi_list_cut_sublist(list, &sublist);
}

archi_status_t
archi_list_remove_nodes(
        archi_list_t *list,

        archi_list_node_func_t which_fn,
        void *which_fn_data,

        archi_list_act_func_t free_fn,
        void *free_fn_data,

        bool start_from_head,
        size_t limit,

        size_t *num_removed)
{
    if (list == NULL)
        return ARCHI_ERROR_MISUSE;

    if (start_from_head)
    {
        if (list->head == NULL)
            return ARCHI_ERROR_MISUSE;
    }
    else
    {
        if (list->tail == NULL)
            return ARCHI_ERROR_MISUSE;
    }

    archi_list_node_t *node = start_from_head ? list->head : list->tail;
    size_t counter = 0;

    archi_status_t code;

    for (;;) // whole list
    {
        archi_list_node_t *node_prev = node->prev;
        archi_list_node_t *node_next = node->next;

        bool is_head = (node == list->head);
        bool is_tail = (node == list->tail);

        if (which_fn != NULL)
            code = which_fn(node, is_head, is_tail, which_fn_data);
        else
            code = 0;

        if (code < 0) // stop now
            break;
        else if (code == 0) // remove the current node
        {
            node->prev = node->next = NULL;

            // Free node memory if needed
            if (free_fn != NULL)
            {
                code = free_fn(node, free_fn_data);

                if (code > 0)
                    code = ARCHI_ERROR_UNKNOWN;
            }

            // Update list end pointers
            if (is_head && is_tail) // the last node in the list
            {
                list->head = NULL;
                list->tail = NULL;
            }
            else if (is_head)
                list->head = node_next;
            else if (is_tail)
                list->tail = node_prev;

            // Update list internal links
            if (node_prev != NULL)
                node_prev->next = node_next;

            if (node_next != NULL)
                node_next->prev = node_prev;

            // Update the counter
            counter++;
        }
        // else proceed

        if (start_from_head)
        {
            if (is_tail)
            {
                code = 0;
                break;
            }
            else if (node_next == NULL)
            {
                code = 1;
                break;
            }

            node = node_next;
        }
        else
        {
            if (is_head)
            {
                code = 0;
                break;
            }
            else if (node_prev == NULL)
            {
                code = 1;
                break;
            }

            node = node_prev;
        }

        if (limit && (counter == limit))
        {
            code = 1;
            break;
        }
    }

    if (num_removed != NULL)
        *num_removed = counter;

    return code;
}

archi_status_t
archi_list_traverse(
        archi_list_t *list,

        archi_list_node_func_t which_fn,
        void *which_fn_data,

        archi_list_act_func_t act_fn,
        void *act_fn_data,

        bool start_from_head,
        size_t limit,

        size_t *num_counted)
{
    if (list == NULL)
        return ARCHI_ERROR_MISUSE;

    if (start_from_head)
    {
        if (list->head == NULL)
            return ARCHI_ERROR_MISUSE;
    }
    else
    {
        if (list->tail == NULL)
            return ARCHI_ERROR_MISUSE;
    }

    archi_list_node_t *list_head = list->head;
    archi_list_node_t *list_tail = list->tail;

    archi_list_node_t *node = start_from_head ? list_head : list_tail;
    size_t counter = 0;

    archi_status_t code;

    for (;;) // whole list
    {
        bool is_head = (node == list->head);
        bool is_tail = (node == list->tail);

        if (which_fn != NULL)
            code = which_fn(node, is_head, is_tail, which_fn_data);
        else
            code = 0;

        if (code < 0) // stop now
            break;
        else if (code == 0) // process and count the current node
        {
            // Process node if needed
            if (act_fn != NULL)
            {
                code = act_fn(node, act_fn_data);

                if (code > 0)
                    code = ARCHI_ERROR_UNKNOWN;
            }

            // Update the counter
            counter++;
        }
        // else proceed

        if (start_from_head)
        {
            if (is_tail)
            {
                code = 0;
                break;
            }
            else if (node->next == NULL)
            {
                code = 1;
                break;
            }

            node = node->next;
        }
        else
        {
            if (is_head)
            {
                code = 0;
                break;
            }
            else if (node->prev == NULL)
            {
                code = 1;
                break;
            }

            node = node->prev;
        }

        if (limit && (counter == limit))
        {
            code = 1;
            break;
        }
    }

    if (num_counted != NULL)
        *num_counted = counter;

    return code;
}

/*****************************************************************************/

archi_status_t
archi_list_node_func_select_by_name(
        const archi_list_node_t *node,
        bool is_head,
        bool is_tail,
        void *data)
{
    (void) is_head;
    (void) is_tail;

    if ((node == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;
    else if (strcmp(((const archi_list_node_named_t*)node)->name, data) == 0)
        return 0; // select
    else
        return 1; // skip
}

/*****************************************************************************/

archi_status_t
archi_list_act_func_copy_node(
        archi_list_node_t *node,
        void *data)
{
    if (data == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_list_node_t **node_ptr = data;
    *node_ptr = node;

    return 0; // no error
}

archi_status_t
archi_list_act_func_free(
        archi_list_node_t *node,
        void *data)
{
    (void) data;

    free(node);

    return 0; // no error
}

archi_status_t
archi_list_act_func_free_named(
        archi_list_node_t *node,
        void *data)
{
    (void) data;

    if (node != NULL)
    {
        free(((archi_list_node_named_t*)node)->name);
        free(node);
    }

    return 0; // no error
}

