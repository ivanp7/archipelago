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

    if ((list->head == NULL) && (list->tail == NULL))
    {
        if ((where_fn == NULL) || (where_fn(NULL, NULL, 0, true, where_fn_data) == 0))
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
        if (list->head == NULL)
            return ARCHI_ERROR_MISUSE;

        node_prev = list->head->prev;
        node_next = list->head;
    }
    else
    {
        if (list->tail == NULL)
            return ARCHI_ERROR_MISUSE;

        node_prev = list->tail;
        node_next = list->tail->next;
    }

    archi_status_t code;

    for (size_t position = 0;; position++) // whole list
    {
        if (where_fn != NULL)
        {
            archi_list_node_t *prev = node_prev;
            archi_list_node_t *next = node_next;

            // Hide nodes beyond the list borders
            if ((list->head != NULL) && (node_next == list->head))
                prev = NULL;

            if ((list->tail != NULL) && (node_prev == list->tail))
                next = NULL;

            bool is_last;
            if (start_from_head)
                is_last = (next == NULL);
            else
                is_last = (prev == NULL);

            code = where_fn(prev, next, position, is_last, where_fn_data);
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
            if ((list->head != NULL) && (node_next == list->head)) // regardless of node_prev value
                list->head = sublist->head;

            if ((list->tail != NULL) && (node_prev == list->tail)) // regardless of node_next value
                list->tail = sublist->tail;

            break;
        }
        // else proceed

        if (start_from_head)
        {
            if ((node_next == NULL) || ((list->tail != NULL) && (node_prev == list->tail)))
            {
                code = 1;
                break;
            }

            node_prev = node_next;
            node_next = node_next->next;
        }
        else
        {
            if ((node_prev == NULL) || ((list->head != NULL) && (node_next == list->head)))
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

    for (size_t position = 0;; position++) // whole list
    {
        archi_list_node_t *node_prev = node->prev;
        archi_list_node_t *node_next = node->next;

        bool is_head = (node == list->head);
        bool is_tail = (node == list->tail);
        {
            bool is_last;
            if (start_from_head)
                is_last = (node_next == NULL) || (node_next == list->head);
            else
                is_last = (node_prev == NULL) || (node_prev == list->tail);

            if (which_fn != NULL)
                code = which_fn(node, position, is_last, which_fn_data);
            else
                code = 0;
        }

        if (code < 0) // stop now
            break;
        else if (code == 0) // remove the current node
        {
            node->prev = node->next = NULL;

            // Free node memory if needed
            if (free_fn != NULL)
            {
                code = free_fn(node, position, free_fn_data);

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

        if (limit && (counter == limit))
        {
            code = 1;
            break;
        }

        if (start_from_head)
        {
            if (is_tail || (node_next == NULL))
            {
                code = 0;
                break;
            }

            node = node_next;
        }
        else
        {
            if (is_head || (node_prev == NULL))
            {
                code = 0;
                break;
            }

            node = node_prev;
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

    archi_list_node_t *node = start_from_head ? list->head : list->tail;
    size_t counter = 0;

    archi_status_t code;

    for (size_t position = 0;; position++) // whole list
    {
        bool is_head = (node == list->head);
        bool is_tail = (node == list->tail);
        {
            archi_list_node_t *node_prev = node->prev;
            archi_list_node_t *node_next = node->next;

            bool is_last;
            if (start_from_head)
                is_last = (node_next == NULL) || (node_next == list->head);
            else
                is_last = (node_prev == NULL) || (node_prev == list->tail);

            if (which_fn != NULL)
                code = which_fn(node, position, is_last, which_fn_data);
            else
                code = 0;
        }

        if (code < 0) // stop now
            break;
        else if (code == 0) // process and count the current node
        {
            // Process node if needed
            if (act_fn != NULL)
            {
                code = act_fn(node, position, act_fn_data);

                if (code > 0)
                    code = ARCHI_ERROR_UNKNOWN;
            }

            // Update the counter
            counter++;
        }
        // else proceed

        if (limit && (counter == limit))
        {
            code = 1;
            break;
        }

        if (start_from_head)
        {
            if (is_tail || (node->next == NULL))
            {
                code = 0;
                break;
            }

            node = node->next;
        }
        else
        {
            if (is_head || (node->prev == NULL))
            {
                code = 0;
                break;
            }

            node = node->prev;
        }
    }

    if (num_counted != NULL)
        *num_counted = counter;

    return code;
}

/*****************************************************************************/

ARCHI_LIST_LINK_FUNC(archi_list_link_func_select_every_nth)
{
    (void) prev;
    (void) next;
    (void) is_last;

    if (data == NULL)
        return ARCHI_ERROR_MISUSE;

    size_t nth = *(size_t*)data;

    return (position + 1) % (nth + 1) != 0;
}

ARCHI_LIST_NODE_FUNC(archi_list_node_func_select_every_nth)
{
    (void) node;
    (void) is_last;

    if (data == NULL)
        return ARCHI_ERROR_MISUSE;

    size_t nth = *(size_t*)data;

    return (position + 1) % (nth + 1) != 0;
}

ARCHI_LIST_NODE_FUNC(archi_list_node_func_select_by_name)
{
    (void) position;
    (void) is_last;

    const archi_list_node_named_t* nnode = (const archi_list_node_named_t*)node;

    if ((nnode->name == NULL) || (data == NULL))
        return ARCHI_ERROR_MISUSE;

    return strcmp(nnode->name, data) != 0;
}

ARCHI_LIST_ACT_FUNC(archi_list_act_func_extract_node)
{
    (void) position;

    if (data == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_list_node_t **node_ptr = data;
    *node_ptr = node;

    return 0; // no error
}

ARCHI_LIST_ACT_FUNC(archi_list_act_func_free)
{
    (void) position;
    (void) data;

    free(node);

    return 0; // no error
}

ARCHI_LIST_ACT_FUNC(archi_list_act_func_free_named)
{
    (void) position;
    (void) data;

    if (node != NULL)
    {
        free((char*)((archi_list_node_named_t*)node)->name);
        free(node);
    }

    return 0; // no error
}

/*****************************************************************************/

static
const char*
archi_list_node_copy_name(
        const char *name)
{
    size_t name_size = strlen(name) + 1;

    char *copy = malloc(name_size);
    if (copy == NULL)
        return NULL;

    memcpy(copy, name, name_size);
    return copy;
}

ARCHI_CONTAINER_INSERT_FUNC(archi_list_container_insert)
{
    if ((container == NULL) || (key == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_list_container_data_t *list_data = container;

    archi_list_node_named_value_t *node = malloc(sizeof(*node));
    if (node == NULL)
        return ARCHI_ERROR_ALLOC;

    *node = (archi_list_node_named_value_t){0};

    node->base.name = archi_list_node_copy_name(key);
    if (node->base.name == NULL)
    {
        free(node);
        return ARCHI_ERROR_ALLOC;
    }

    node->value = (archi_value_t){
        .ptr = element,
        .num_of = 1,
        .type = ARCHI_VALUE_DATA,
    };

    archi_status_t code = archi_list_insert_node(&list_data->list, (archi_list_node_t*)node,
            NULL, NULL, list_data->insert_to_head);
    if (code != 0)
    {
        archi_list_act_func_free_named((archi_list_node_t*)node, 0, NULL);
        return code;
    }

    return 0;
}

ARCHI_CONTAINER_REMOVE_FUNC(archi_list_container_remove)
{
    if (container == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_list_container_data_t *list_data = container;

    archi_list_node_named_value_t *node = NULL;
    size_t num_counted = 0;

    archi_status_t code;

    code = archi_list_traverse(&list_data->list,
            archi_list_node_func_select_by_name, (void*)key, archi_list_act_func_extract_node, &node,
            list_data->traverse_from_head, 1, &num_counted);

    if (code != 1)
    {
        if (code == 0)
            return 1;

        return code;
    }
    else if (num_counted != 1)
        return 1; // not found

    if (!archi_list_cut_node(&list_data->list, (archi_list_node_t*)node))
        return ARCHI_ERROR_UNKNOWN; // this should not happen

    archi_list_act_func_free_named((archi_list_node_t*)node, 0, NULL);

    if (element != NULL)
        *element = node->value.ptr;

    return 0;
}

ARCHI_CONTAINER_EXTRACT_FUNC(archi_list_container_extract)
{
    if (container == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_list_container_data_t *list_data = container;

    archi_list_node_named_value_t *node = NULL;
    size_t num_counted = 0;

    archi_status_t code;

    code = archi_list_traverse(&list_data->list,
            archi_list_node_func_select_by_name, (void*)key, archi_list_act_func_extract_node, &node,
            list_data->traverse_from_head, 1, &num_counted);

    if (code != 1)
    {
        if (code == 0)
            return 1;

        return code;
    }
    else if (num_counted != 1)
        return 1; // not found

    if (element != NULL)
        *element = node->value.ptr;

    return 0;
}

struct archi_list_container_traverse_act_func_data {
    archi_container_element_func_t func;
    void *func_data;
};

static
ARCHI_LIST_ACT_FUNC(archi_list_container_traverse_act_func)
{
    (void) position;

    struct archi_list_container_traverse_act_func_data *data_ptr = data;
    const archi_list_node_named_value_t *node_ptr = (const archi_list_node_named_value_t*)node;

    return data_ptr->func(node_ptr->base.name, node_ptr->value.ptr, data_ptr->func_data);
}

ARCHI_CONTAINER_TRAVERSE_FUNC(archi_list_container_traverse)
{
    if (container == NULL)
        return ARCHI_ERROR_MISUSE;

    archi_list_container_data_t *list_data = container;

    struct archi_list_container_traverse_act_func_data data = {
        .func = func,
        .func_data = func_data,
    };

    return archi_list_traverse(&list_data->list, NULL, NULL,
            archi_list_container_traverse_act_func, &data,
            list_data->traverse_from_head, 0, NULL);
}

const archi_container_interface_t archi_list_container_interface = {
    .insert_fn = archi_list_container_insert,
    .remove_fn = archi_list_container_remove,
    .extract_fn = archi_list_container_extract,
    .traverse_fn = archi_list_container_traverse,
};

