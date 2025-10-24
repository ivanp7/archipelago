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
 * @brief Operations on hashmaps.
 */

#include "archi/ds_hashmap/api/hashmap.fun.h"
#include "archipelago/base/ref_count.fun.h"
#include "archipelago/util/alloc.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for strcmp()

size_t
archi_hash(
        const char *string)
{
    if (string == NULL)
        return 0;

    size_t hash = 5381;

    char c;
    while ((c = *string++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return hash;
}

/*****************************************************************************/

struct archi_hashmap_node;

struct archi_hashmap_node {
    char *key;             ///< Key.
    archi_pointer_t value; ///< Value.

    size_t hash; ///< Hash of the key (modulo capacity).
    struct archi_hashmap_node *hash_next; ///< Next node in the list of nodes with the same hash.
    struct archi_hashmap_node *hash_prev; ///< Next node in the list of nodes with the same hash.

    struct archi_hashmap_node *chrono_next; ///< Chronologically next inserted node.
    struct archi_hashmap_node *chrono_prev; ///< Chronologically previous inserted node.
};

struct archi_hashmap {
    size_t capacity; ///< Size of '.nodes' array.
    size_t size; ///< Total number of nodes.

    struct archi_hashmap_node *chrono_first; ///< The chronologically first inserted node.
    struct archi_hashmap_node *chrono_last;  ///< The chronologically last inserted node.

    struct archi_hashmap_node *nodes[]; ///< Array of nodes by hash.
};

archi_hashmap_t
archi_hashmap_alloc(
        archi_hashmap_alloc_params_t params,
        archi_status_t *code)
{
    if (params.capacity == 0)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    size_t size = ARCHI_SIZEOF_FLEXIBLE(struct archi_hashmap, nodes, params.capacity);

    archi_hashmap_t hashmap = malloc(size);
    if (hashmap == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return NULL;
    }

    hashmap->capacity = params.capacity;
    hashmap->size = 0;

    hashmap->chrono_first = hashmap->chrono_last = NULL;

    for (size_t i = 0; i < params.capacity; i++)
        hashmap->nodes[i] = NULL;

    if (code != NULL)
        *code = 0;

    return hashmap;
}

static
ARCHI_HASHMAP_TRAV_KV_FUNC(archi_hashmap_free_unset)
{
    (void) key;
    (void) value;
    (void) index;
    (void) data;

    return (archi_hashmap_trav_action_t){.type = ARCHI_HASHMAP_TRAV_UNSET};
}

void
archi_hashmap_free(
        archi_hashmap_t hashmap)
{
    if (hashmap == NULL)
        return;

    archi_hashmap_traverse(hashmap, false, archi_hashmap_free_unset, NULL);
    free(hashmap);
}

archi_pointer_t
archi_hashmap_get(
        archi_hashmap_t hashmap,

        const char *key,
        archi_status_t *code)
{
    if (hashmap == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return (archi_pointer_t){0};
    }

    // Find the node
    struct archi_hashmap_node *node;
    {
        size_t hash = archi_hash(key);
        hash %= hashmap->capacity;

        node = hashmap->nodes[hash];
        while (node != NULL)
        {
            if (strcmp(key, node->key) == 0)
                break;

            node = node->hash_next;
        }
    }

    if (node != NULL)
    {
        if (code != NULL)
            *code = 0;

        return node->value;
    }
    else
    {
        if (code != NULL)
            *code = 1;

        return (archi_pointer_t){0};
    }
}

archi_status_t
archi_hashmap_set(
        archi_hashmap_t hashmap,

        const char *key,
        archi_pointer_t value,
        archi_hashmap_set_params_t params)
{
    if (hashmap == NULL)
        return ARCHI_STATUS_EMISUSE;
    else if (key == NULL)
        return ARCHI_STATUS_EMISUSE;

    size_t hash = archi_hash(key);
    hash %= hashmap->capacity;

    // Find the node
    struct archi_hashmap_node *node;
    {
        node = hashmap->nodes[hash];
        while (node != NULL)
        {
            if (strcmp(key, node->key) == 0)
                break;

            node = node->hash_next;
        }
    }

    if (node == NULL)
    {
        if (!params.insertion_allowed)
            return 1; // the key doesn't exist and key insertion is not allowed

        // Insert the new node
        node = malloc(sizeof(*node));
        if (node == NULL)
            return ARCHI_STATUS_ENOMEMORY;

        char *key_copy = archi_copy_string(key);
        if (key_copy == NULL)
        {
            free(node);
            return ARCHI_STATUS_ENOMEMORY;
        }

        *node = (struct archi_hashmap_node){
            .key = key_copy,
            .value = value,
            .hash = hash,
            .hash_next = hashmap->nodes[hash],
            .chrono_prev = hashmap->chrono_last,
        };

        if (node->hash_next != NULL)
            node->hash_next->hash_prev = node;

        if (hashmap->chrono_first == NULL)
            hashmap->chrono_first = node;
        else // hashmap->chrono_last != NULL
            hashmap->chrono_last->chrono_next = node;

        hashmap->chrono_last = hashmap->nodes[hash] = node;

        hashmap->size++;

        archi_reference_count_increment(value.ref_count);
    }
    else
    {
        if (!params.update_allowed)
            return 2; // the key exists and value updating is not allowed

        if (params.set_fn != NULL)
        {
            if (!params.set_fn(key, node->value, params.set_fn_data))
                return 3; // the key exists and the key-value function returned false
        }

        archi_reference_count_increment(value.ref_count);       // increment first to prevent unwanted deallocation
                                                                // if value == node->value,
        archi_reference_count_decrement(node->value.ref_count); // then decrement

        node->value = value;
    }

    return 0;
}

static
void
archi_hashmap_remove_node(
        archi_hashmap_t hashmap,
        struct archi_hashmap_node *node)
{
    // Remove the node from the list of nodes with identical hash keys
    if (node->hash_prev != NULL)
        node->hash_prev->hash_next = node->hash_next;
    else
        hashmap->nodes[node->hash] = node->hash_next;

    if (node->hash_next != NULL)
        node->hash_next->hash_prev = node->hash_prev;

    // Remove the node from the chronological insertion order list
    if (node->chrono_prev != NULL)
        node->chrono_prev->chrono_next = node->chrono_next;
    else
        hashmap->chrono_first = node->chrono_next;

    if (node->chrono_next != NULL)
        node->chrono_next->chrono_prev = node->chrono_prev;
    else
        hashmap->chrono_last = node->chrono_prev;

    hashmap->size--;
}

archi_status_t
archi_hashmap_unset(
        archi_hashmap_t hashmap,

        const char *key,
        archi_hashmap_unset_params_t params)
{
    if (hashmap == NULL)
        return ARCHI_STATUS_EMISUSE;
    else if (key == NULL)
        return ARCHI_STATUS_EMISUSE;

    // Find the node
    struct archi_hashmap_node *node;
    {
        size_t hash = archi_hash(key);
        hash %= hashmap->capacity;

        node = hashmap->nodes[hash];
        while (node != NULL)
        {
            if (strcmp(key, node->key) == 0)
                break;

            node = node->hash_next;
        }
    }

    if (node == NULL)
        return 1; // the key does not exist

    if (params.unset_fn != NULL)
    {
        if (!params.unset_fn(key, node->value, params.unset_fn_data))
            return 3; // the key exists and the key-value function returned false
    }

    // Remove the node from lists
    archi_hashmap_remove_node(hashmap, node);

    // Decrement the value reference counter and destroy the node
    archi_reference_count_decrement(node->value.ref_count);

    free(node->key);
    free(node);

    return 0;
}

archi_status_t
archi_hashmap_traverse(
        archi_hashmap_t hashmap,

        bool first_to_last,
        archi_hashmap_trav_kv_func_t trav_fn,
        void *trav_fn_data)
{
    if (hashmap == NULL)
        return ARCHI_STATUS_EMISUSE;
    else if (trav_fn == NULL)
        return ARCHI_STATUS_EMISUSE;

    struct archi_hashmap_node *node = first_to_last ? hashmap->chrono_first : hashmap->chrono_last;
    size_t index = 0;

    while (node != NULL)
    {
        struct archi_hashmap_node *following = first_to_last ? node->chrono_next : node->chrono_prev;

        archi_hashmap_trav_action_t action = trav_fn(node->key, node->value, index++, trav_fn_data);
        switch (action.type)
        {
            case ARCHI_HASHMAP_TRAV_KEEP:
                // do nothing
                break;

            case ARCHI_HASHMAP_TRAV_SET:
                archi_reference_count_increment(action.new_value.ref_count); // increment first to prevent unwanted deallocation
                                                                             // if action.new_value == node->value,
                archi_reference_count_decrement(node->value.ref_count);      // then decrement

                node->value = action.new_value;
                break;

            case ARCHI_HASHMAP_TRAV_UNSET:
                archi_hashmap_remove_node(hashmap, node);

                archi_reference_count_decrement(node->value.ref_count);

                free(node->key);
                free(node);
                break;

            default:
                return ARCHI_STATUS_EVALUE;
        }

        if (action.interrupt)
            return 1;

        node = following;
    }

    return 0;
}

size_t
archi_hashmap_size(
        archi_hashmap_t hashmap)
{
    if (hashmap == NULL)
        return 0;

    return hashmap->size;
}

