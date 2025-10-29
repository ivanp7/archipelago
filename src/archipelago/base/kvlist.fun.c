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

#include "archipelago/base/kvlist.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/base/ref_count.fun.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()

archi_kvlist_t*
archi_kvlist_alloc_node(
        const char *key)
{
    archi_kvlist_t *node = malloc(sizeof(*node));
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

    *node = (archi_kvlist_t){
        .key = key,
    };

    return node;
}

void
archi_kvlist_free_node(
        archi_kvlist_t *node)
{
    if (node == NULL)
        return;

    free((char*)node->key);
    free(node);
}

void
archi_kvlist_free(
        archi_kvlist_t *list)
{
    while (list != NULL)
    {
        archi_kvlist_t *next = list->next;

        free((char*)list->key);
        free(list);

        list = next;
    }
}

archi_kvlist_t*
archi_kvlist_copy(
        const archi_kvlist_t *list,
        archi_kvlist_t **out_tail)
{
    if (list == NULL)
    {
        if (out_tail != NULL)
            *out_tail = NULL;

        return NULL;
    }

    archi_kvlist_t *head = NULL, *tail = NULL;

    for (; list != NULL; list = list->next)
    {
        archi_kvlist_t *node = archi_kvlist_alloc_node(list->key);
        if (node == NULL)
            goto failure;

        node->value = list->value;

        if (tail != NULL)
        {
            tail->next = node;
            tail = node;
        }
        else
            head = tail = node;
    }

    if (out_tail != NULL)
        *out_tail = tail;

    return head;

failure:
    archi_kvlist_free(head);

    if (out_tail != NULL)
        *out_tail = NULL;

    return NULL;
}

archi_kvlist_rc_t*
archi_kvlist_copy_with_refcounts(
        const archi_kvlist_t *list,
        archi_reference_count_t ref_count,
        archi_kvlist_rc_t **out_tail)
{
    if (list == NULL)
    {
        if (out_tail != NULL)
            *out_tail = NULL;

        return NULL;
    }

    archi_kvlist_rc_t *head = NULL, *tail = NULL;

    for (; list != NULL; list = list->next)
    {
        archi_kvlist_rc_t *node = archi_kvlist_rc_alloc_node(list->key);
        if (node == NULL)
            goto failure;

        node->value = archi_pointer_with_refcount(list->value, ref_count);

        if (tail != NULL)
        {
            tail->next = node;
            tail = node;
        }
        else
            head = tail = node;
    }

    if (out_tail != NULL)
        *out_tail = tail;

    return head;

failure:
    archi_kvlist_rc_free(head, false);

    if (out_tail != NULL)
        *out_tail = NULL;

    return NULL;
}

bool
archi_kvlist_node(
        const archi_kvlist_t *list,
        const char *key,
        const archi_kvlist_t **out_node)
{
    if (key == NULL)
    {
        if (out_node != NULL)
            *out_node = NULL;

        return false;
    }

    for (; list != NULL; list = list->next)
    {
        if (ARCHI_STRING_COMPARE(list->key, ==, key))
        {
            if (out_node != NULL)
                *out_node = list;

            return true;
        }
    }

    if (out_node != NULL)
        *out_node = NULL;

    return false;
}

const archi_kvlist_t*
archi_kvlist_tail(
        const archi_kvlist_t *list)
{
    if (list == NULL)
        return NULL;

    while (list->next != NULL)
        list = list->next;

    return list;
}

/*****************************************************************************/

archi_kvlist_rc_t*
archi_kvlist_rc_alloc_node(
        const char *key)
{
    archi_kvlist_rc_t *node = malloc(sizeof(*node));
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

    *node = (archi_kvlist_rc_t){
        .key = key,
    };

    return node;
}

void
archi_kvlist_rc_free_node(
        archi_kvlist_rc_t *node)
{
    if (node == NULL)
        return;

    free((char*)node->key);
    free(node);
}

void
archi_kvlist_rc_free(
        archi_kvlist_rc_t *list,
        bool decrement_ref_count)
{
    while (list != NULL)
    {
        archi_kvlist_rc_t *next = list->next;

        if (decrement_ref_count)
            archi_reference_count_decrement(list->value.ref_count);
        free((char*)list->key);
        free(list);

        list = next;
    }
}

archi_kvlist_rc_t*
archi_kvlist_rc_copy(
        const archi_kvlist_rc_t *list,
        archi_kvlist_rc_t **out_tail)
{
    if (list == NULL)
    {
        if (out_tail != NULL)
            *out_tail = NULL;

        return NULL;
    }

    archi_kvlist_rc_t *head = NULL, *tail = NULL;

    for (; list != NULL; list = list->next)
    {
        archi_kvlist_rc_t *node = archi_kvlist_rc_alloc_node(list->key);
        if (node == NULL)
            goto failure;

        node->value = list->value;

        if (tail != NULL)
        {
            tail->next = node;
            tail = node;
        }
        else
            head = tail = node;
    }

    if (out_tail != NULL)
        *out_tail = tail;

    return head;

failure:
    archi_kvlist_rc_free(head, false);

    if (out_tail != NULL)
        *out_tail = NULL;

    return NULL;
}

bool
archi_kvlist_rc_node(
        const archi_kvlist_rc_t *list,
        const char *key,
        const archi_kvlist_rc_t **out_node)
{
    if (key == NULL)
    {
        if (out_node != NULL)
            *out_node = NULL;

        return false;
    }

    for (; list != NULL; list = list->next)
    {
        if (ARCHI_STRING_COMPARE(list->key, ==, key))
        {
            if (out_node != NULL)
                *out_node = list;

            return true;
        }
    }

    if (out_node != NULL)
        *out_node = NULL;

    return false;
}

const archi_kvlist_rc_t*
archi_kvlist_rc_tail(
        const archi_kvlist_rc_t *list)
{
    if (list == NULL)
        return NULL;

    while (list->next != NULL)
        list = list->next;

    return list;
}

