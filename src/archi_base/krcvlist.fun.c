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
 * @brief Operations on key-value lists with reference counted values.
 */

#include "archi_base/krcvlist.fun.h"
#include "archi_base/rcpointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/ref_count.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


archi_krcvlist_t*
archi_krcvlist_node_alloc(
        const char *key)
{
    archi_krcvlist_t *node = malloc(sizeof(*node));
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

    *node = (archi_krcvlist_t){
        .n.key = key,
    };

    return node;
}

void
archi_krcvlist_node_free(
        archi_krcvlist_t *node)
{
    if (node == NULL)
        return;

    free((char*)node->n.key);
    free(node);
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

    for (; head != NULL; head = (archi_krcvlist_t*)head->n.next)
    {
        archi_krcvlist_t *node = archi_krcvlist_node_alloc(head->n.key);
        if (node == NULL)
            goto failure;

        node->value = head->value;

        if (copy_tail != NULL)
        {
            copy_tail->n.next = (archi_kvlist_base_t*)node;
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

archi_krcvlist_t*
archi_krcvlist_copy_kvlist(
        const archi_kvlist_t *head,
        archi_reference_count_t ref_count,
        archi_krcvlist_t **out_tail)
{
    if (head == NULL)
    {
        if (out_tail != NULL)
            *out_tail = NULL;
        return NULL;
    }

    archi_krcvlist_t *copy_head = NULL, *copy_tail = NULL;

    for (; head != NULL; head = (archi_kvlist_t*)head->n.next)
    {
        archi_krcvlist_t *node = archi_krcvlist_node_alloc(head->n.key);
        if (node == NULL)
            goto failure;

        node->value = archi_rcpointer(head->value,
                !ARCHI_POINTER_IS_NULL(head->value) ? ref_count : NULL);

        if (copy_tail != NULL)
        {
            copy_tail->n.next = (archi_kvlist_base_t*)node;
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
archi_krcvlist_free(
        archi_krcvlist_t *head,
        bool decrement_ref_counts)
{
    while (head != NULL)
    {
        archi_krcvlist_t *next = (archi_krcvlist_t*)head->n.next;
        {
            if (decrement_ref_counts)
                archi_reference_count_decrement(head->value.ref_count);

            archi_krcvlist_node_free(head);
        }
        head = next;
    }
}

