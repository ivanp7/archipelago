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
 * @brief Operations on key-value lists.
 */

#include "archi_base/kvlist.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/ref_count.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


archi_kvlist_t*
archi_kvlist_node_alloc(
        const char *key,
        bool alloc_ref_count)
{
    archi_kvlist_t *node = malloc(!alloc_ref_count ?
            sizeof(archi_kvlist_t) : sizeof(archi_krcvlist_t));
    if (node == NULL)
        return NULL;

    if (key != NULL)
    {
        key = archi_string_copy(key);
        if (key == NULL)
        {
            free(node);
            return NULL;
        }
    }

    if (!alloc_ref_count)
        *node = (archi_kvlist_t){
            .key = key,
        };
    else
        *(archi_krcvlist_t*)node = (archi_krcvlist_t){
            .key = key,
        };

    return node;
}

void
archi_kvlist_node_free(
        archi_kvlist_t *node)
{
    if (node == NULL)
        return;

    free((char*)node->key);
    free(node);
}

/*****************************************************************************/

archi_kvlist_t*
archi_kvlist_copy(
        const archi_kvlist_t *head,
        bool alloc_ref_count,
        archi_reference_count_t ref_count,
        archi_kvlist_t **out_tail)
{
    if (head == NULL)
    {
        if (out_tail != NULL)
            *out_tail = NULL;
        return NULL;
    }

    archi_kvlist_t *copy_head = NULL, *copy_tail = NULL;

    for (; head != NULL; head = head->next)
    {
        archi_kvlist_t *node = archi_kvlist_node_alloc(head->key, alloc_ref_count);
        if (node == NULL)
            goto failure;

        if (!alloc_ref_count)
            node->value = head->value;
        else
            ((archi_krcvlist_t*)node)->value = archi_rcpointer(head->value, ref_count);

        if (copy_tail != NULL)
        {
            copy_tail->next = node;
            copy_tail = node;
        }
        else
            copy_head = copy_tail = node;
    }

    if (out_tail != NULL)
        *out_tail = copy_tail;
    return copy_head;

failure:
    archi_kvlist_free(copy_head);

    if (out_tail != NULL)
        *out_tail = NULL;
    return NULL;
}

archi_krcvlist_t*
archi_krcvlist_copy(
        const archi_krcvlist_t *head,
        archi_krcvlist_t **out_tail)
{
    if (head == NULL)
    {
        if (out_tail != NULL)
            *out_tail = NULL;
        return NULL;
    }

    archi_krcvlist_t *copy_head = NULL, *copy_tail = NULL;

    for (; head != NULL; head = head->next)
    {
        archi_krcvlist_t *node = (archi_krcvlist_t*)archi_kvlist_node_alloc(head->key, true);
        if (node == NULL)
            goto failure;

        node->value = head->value;

        if (copy_tail != NULL)
        {
            copy_tail->next = node;
            copy_tail = node;
        }
        else
            copy_head = copy_tail = node;
    }

    if (out_tail != NULL)
        *out_tail = copy_tail;
    return copy_head;

failure:
    archi_krcvlist_free(copy_head, false);

    if (out_tail != NULL)
        *out_tail = NULL;
    return NULL;
}

void
archi_kvlist_free(
        archi_kvlist_t *head)
{
    archi_krcvlist_free((archi_krcvlist_t*)head, false);
}

void
archi_krcvlist_free(
        archi_krcvlist_t *head,
        bool decrement_ref_counts)
{
    while (head != NULL)
    {
        archi_krcvlist_t *next = (archi_krcvlist_t*)head->next;
        {
            if (decrement_ref_counts)
                archi_reference_count_decrement(head->ref_count);
            archi_kvlist_node_free(&head->n);
        }
        head = next;
    }
}

const archi_kvlist_t*
archi_kvlist_tail(
        const archi_kvlist_t *head,
        size_t *out_length)
{
    if (head == NULL)
    {
        if (out_length != NULL)
            *out_length = 0;
        return NULL;
    }

    size_t length = 1;
    while (head->next != NULL)
    {
        head = head->next;
        length++;
    }

    if (out_length != NULL)
        *out_length = length;
    return head;
}

const archi_kvlist_t*
archi_kvlist_nth_node(
        const archi_kvlist_t *head,
        size_t index)
{
    while ((head != NULL) && (index > 0))
    {
        head = head->next;
        index--;
    }

    return head;
}

const archi_kvlist_t*
archi_kvlist_find(
        const archi_kvlist_t *head,
        const char *key,
        archi_string_comp_func_t comp_fn,
        size_t *out_index)
{
    if (comp_fn == NULL)
        comp_fn = strcmp;

    size_t index = 0;
    for (; head != NULL; head = head->next, index++)
    {
        if (((key != NULL) && (head->key != NULL) &&
                    (comp_fn(key, head->key) == 0)) ||
                ((key == NULL) && (head->key == NULL)))
        {
            if (out_index != NULL)
                *out_index = index;
            return head;
        }
    }

    if (out_index != NULL)
        *out_index = index;
    return NULL;
}

