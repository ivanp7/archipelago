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
 * @brief Operations on hashmaps.
 */

#include "archi/hashmap/api/hashmap.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/util/size.def.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()


struct archi_hashmap_node;

struct archi_hashmap_node {
    char *key;             ///< Key.
    archi_rcpointer_t value; ///< Value.

    size_t hash; ///< Hash of the key (modulo capacity).
    struct archi_hashmap_node *hash_next; ///< Next node in the list of nodes with the same hash.
    struct archi_hashmap_node *hash_prev; ///< Next node in the list of nodes with the same hash.

    struct archi_hashmap_node *chrono_next; ///< Chronologically next inserted node.
    struct archi_hashmap_node *chrono_prev; ///< Chronologically previous inserted node.
};

struct archi_hashmap {
    archi_hashmap_hash_func_t hash_fn; ///< Hash function.

    size_t capacity; ///< Size of the array of nodes.
    size_t num_nodes; ///< Total number of nodes.

    struct archi_hashmap_node *chrono_first; ///< The chronologically first inserted node.
    struct archi_hashmap_node *chrono_last;  ///< The chronologically last inserted node.

    struct archi_hashmap_node *nodes[]; ///< Array of nodes indexed by hash.
};

archi_hashmap_t
archi_hashmap_alloc(
        archi_hashmap_hash_func_t hash_fn,
        archi_hashmap_alloc_params_t params,
        ARCHI_ERROR_PARAM_DECL)
{
    if (params.capacity == 0)
        params.capacity = ARCHI_HASHMAP_DEFAULT_CAPACITY;

    if (hash_fn == NULL)
        hash_fn = archi_string_hash;

    archi_hashmap_t hashmap = malloc(ARCHI_SIZEOF_FLEXIBLE(
                struct archi_hashmap, nodes, params.capacity));
    if (hashmap == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate hashmap object");
        return NULL;
    }

    hashmap->hash_fn = hash_fn;

    hashmap->capacity = params.capacity;
    hashmap->num_nodes = 0;

    hashmap->chrono_first = hashmap->chrono_last = NULL;

    for (size_t i = 0; i < params.capacity; i++)
        hashmap->nodes[i] = NULL;

    ARCHI_ERROR_RESET();
    return hashmap;
}

void
archi_hashmap_free(
        archi_hashmap_t hashmap)
{
    if (hashmap == NULL)
        return;

    archi_hashmap_traverse(hashmap, false, archi_hashmap_trav_kv__unset_all, NULL, NULL);
    free(hashmap);
}

bool
archi_hashmap_get(
        archi_hashmap_t hashmap,
        const char *key,
        archi_rcpointer_t *value,
        ARCHI_ERROR_PARAM_DECL)
{
    if (hashmap == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "hashmap is NULL");
        return false;
    }

    // Find the node
    struct archi_hashmap_node *node;
    {
        size_t hash = hashmap->hash_fn(key);
        hash %= hashmap->capacity;

        node = hashmap->nodes[hash];
        while (node != NULL)
        {
            if (ARCHI_STRING_COMPARE(key, ==, node->key))
                break;

            node = node->hash_next;
        }
    }

    ARCHI_ERROR_RESET();

    if (node != NULL)
    {
        if (value != NULL)
            *value = node->value;
        return true;
    }
    else
        return false;
}

bool
archi_hashmap_set(
        archi_hashmap_t hashmap,
        const char *key,
        archi_rcpointer_t value,
        archi_hashmap_set_params_t params,
        ARCHI_ERROR_PARAM_DECL)
{
    if (hashmap == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "hashmap is NULL");
        return false;
    }
    else if (key == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "hashmap key is NULL");
        return false;
    }

    size_t hash = hashmap->hash_fn(key);
    hash %= hashmap->capacity;

    // Find the node
    struct archi_hashmap_node *node;
    {
        node = hashmap->nodes[hash];
        while (node != NULL)
        {
            if (ARCHI_STRING_COMPARE(key, ==, node->key))
                break;

            node = node->hash_next;
        }
    }

    if (node == NULL) // the key does not exist, need to insert new node
    {
        if (!params.insertion_allowed)
        {
            ARCHI_ERROR_RESET();
            return false;
        }

        // Insert the new node
        node = malloc(sizeof(*node));
        if (node == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate hashmap node");
            return false;
        }

        char *key_copy = archi_string_copy(key);
        if (key_copy == NULL)
        {
            free(node);
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate key copy");
            return false;
        }

        value = archi_rcpointer_own(value, ARCHI_ERROR_PARAM);
        if (!value.attr)
        {
            free(key_copy);
            free(node);
            return false;
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
        hashmap->num_nodes++;
    }
    else // the key exists, can just update the value of the current node
    {
        if (!params.update_allowed || ((params.set_fn != NULL) &&
                    !params.set_fn(key, node->value, params.set_fn_data)))
        {
            ARCHI_ERROR_RESET();
            return false;
        }

        value = archi_rcpointer_own_disown(value, node->value, ARCHI_ERROR_PARAM);
        if (!value.attr)
            return false;

        node->value = value;
    }

    ARCHI_ERROR_RESET();
    return true;
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

    hashmap->num_nodes--;
}

bool
archi_hashmap_unset(
        archi_hashmap_t hashmap,
        const char *key,
        archi_hashmap_unset_params_t params,
        ARCHI_ERROR_PARAM_DECL)
{
    if (hashmap == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "hashmap is NULL");
        return false;
    }
    else if (key == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "hashmap key is NULL");
        return false;
    }

    // Find the node
    struct archi_hashmap_node *node;
    {
        size_t hash = hashmap->hash_fn(key);
        hash %= hashmap->capacity;

        node = hashmap->nodes[hash];
        while (node != NULL)
        {
            if (ARCHI_STRING_COMPARE(key, ==, node->key))
                break;

            node = node->hash_next;
        }
    }

    if ((node == NULL) || ((params.unset_fn != NULL) &&
                !params.unset_fn(key, node->value, params.unset_fn_data)))
    {
        ARCHI_ERROR_RESET();
        return false;
    }

    // Remove the node from lists
    archi_hashmap_remove_node(hashmap, node);

    // Decrement the value reference counter and destroy the node
    archi_rcpointer_disown(node->value);

    free(node->key);
    free(node);

    ARCHI_ERROR_RESET();
    return true;
}

bool
archi_hashmap_traverse(
        archi_hashmap_t hashmap,
        bool first_to_last,
        archi_hashmap_trav_kv_func_t trav_fn,
        void *trav_fn_data,
        ARCHI_ERROR_PARAM_DECL)
{
    if (hashmap == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "hashmap is NULL");
        return false;
    }
    else if (trav_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "hashmap traversal function is NULL");
        return false;
    }

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
                action.new_value = archi_rcpointer_own_disown(action.new_value, node->value,
                        ARCHI_ERROR_PARAM);
                if (!action.new_value.attr)
                    return false;

                node->value = action.new_value;
                break;

            case ARCHI_HASHMAP_TRAV_UNSET:
                archi_hashmap_remove_node(hashmap, node);

                archi_rcpointer_disown(node->value);

                free(node->key);
                free(node);
                break;

            default:
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "hashmap traversal function returned unknown action type");
                return false;
        }

        if (action.interrupt)
        {
            ARCHI_ERROR_RESET();
            return false;
        }

        node = following;
    }

    ARCHI_ERROR_RESET();
    return true;
}

size_t
archi_hashmap_num_elements(
        archi_hashmap_t hashmap)
{
    if (hashmap == NULL)
        return 0;

    return hashmap->num_nodes;
}

size_t
archi_hashmap_capacity(
        archi_hashmap_t hashmap)
{
    if (hashmap == NULL)
        return 0;

    return hashmap->capacity;
}

/*****************************************************************************/

ARCHI_HASHMAP_TRAV_KV_FUNC(archi_hashmap_trav_kv__unset_all)
{
    (void) key;
    (void) value;
    (void) index;
    (void) data;

    return (archi_hashmap_trav_action_t){.type = ARCHI_HASHMAP_TRAV_UNSET};
}

