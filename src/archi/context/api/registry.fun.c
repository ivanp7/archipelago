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
 * @brief Context registry operations.
 */

#include "archi/context/api/registry.fun.h"
#include "archi/context/api/interface.fun.h"
#include "archi/context/api/callback.fun.h"
#include "archi/context/api/tag.def.h"
#include "archi_base/kvlist.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/tag.def.h"


struct archi_context_params_info {
    const archi_krcvlist_t *params; ///< Concatenated parameter list.

    archi_krcvlist_t *sparams_head; ///< Head node of the static parameter list.
    archi_krcvlist_t *sparams_tail; ///< Tail node of the static parameter list.
};

static
struct archi_context_params_info
archi_context_params_concatenate(
        archi_context_t registry,
        const char *dparams_key,
        archi_krcvlist_t *sparams,
        ARCHI_ERROR_PARAM_DECL)
{
    struct archi_context_params_info params_info = {0};

    // Obtain the 'dynamic' parameter list
    archi_krcvlist_t *dparams = NULL;

    if (dparams_key != NULL)
    {
        // Obtain the parameter list context
        ARCHI_ERROR_VAR(error);

        archi_context_t dparams_context = archi_context_registry_get(registry, dparams_key, &error);
        ARCHI_ERROR_ASSIGN(error);

        if (error.code != 0)
            return params_info;

        archi_rcpointer_t dparams_ptr = archi_context_data(dparams_context);
        if (!archi_pointer_attr_compatible(dparams_ptr.attr,
                    archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__KRCVLIST)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "parameter list context (key = '%s') is not a reference-counted key-value list",
                    dparams_key);
            return params_info;
        }

        dparams = dparams_ptr.ptr;
    }

    // Append the 'dynamic' list to the 'static' list
    archi_krcvlist_t *sparams_tail = (archi_krcvlist_t*)archi_kvlist_tail((archi_kvlist_t*)sparams, NULL);
    if (sparams_tail != NULL)
        sparams_tail->next = dparams;

    // Prepare the parameter list
    params_info.params = (sparams != NULL) ? sparams : dparams;
    params_info.sparams_head = sparams;
    params_info.sparams_tail = sparams_tail;

    ARCHI_ERROR_RESET();
    return params_info;
}

static
void
archi_context_params_finalize_concatenation(
        struct archi_context_params_info params_info)
{
    // Split the 'dynamic' list from the 'static' list
    if (params_info.sparams_tail != NULL)
        params_info.sparams_tail->next = NULL;
}

///////////////////////////////////////////////////////////////////////////////

archi_context_t
archi_context_registry_get(
        archi_context_t registry,
        const char *key,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context key is empty");
        return NULL;
    }

    archi_rcpointer_t context = {0};

    archi_context_get(registry, (archi_context_slot_t){.name = key},
            (archi_context_callback_t){.function = archi_context_callback__getter, .data = &context},
            ARCHI_ERROR_PARAM);

    if (!context.attr)
        return NULL;
    else if (!archi_pointer_attr_compatible(context.attr,
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__CONTEXT)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "object (key = '%s') stored in context registry is not a context", key);
        return NULL;
    }

    return context.ptr;
}

bool
archi_context_registry_contains(
        archi_context_t registry,
        const char *key)
{
    return archi_context_registry_get(registry, key, NULL) != NULL;
}

bool
archi_context_registry_key_available(
        archi_context_t registry,
        const char *key)
{
    ARCHI_ERROR_VAR(error);

    archi_context_registry_get(registry, key, &error);

    return error.code == ARCHI__EKEY;
}

void
archi_context_registry_delete(
        archi_context_t registry,
        const char *key,
        ARCHI_ERROR_PARAM_DECL)
{
    // Delete the context
    archi_context_unset(registry, (archi_context_slot_t){.name = key},
            ARCHI_ERROR_PARAM);
}

void
archi_context_registry_alias(
        archi_context_t registry,
        const char *key,
        const char *original_key,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "alias key is empty");
        return;
    }

    // Create the context alias
    archi_context_set_from_get(registry, (archi_context_slot_t){.name = key},
            registry, (archi_context_slot_t){.name = original_key}, false,
            ARCHI_ERROR_PARAM);
}

void
archi_context_registry_insert(
        archi_context_t registry,
        const char *key,
        archi_context_t context,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "inserted key is empty");
        return;
    }
    else if (context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "inserted context is NULL");
        return;
    }

    // Insert the context
    archi_rcpointer_t context_ptr = {
        .ptr = context,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__CONTEXT),
        .ref_count = archi_context_data(context).ref_count,
    };

    archi_context_set(registry, (archi_context_slot_t){.name = key}, context_ptr,
            ARCHI_ERROR_PARAM);
}

archi_context_t
archi_context_registry_create(
        archi_context_t registry,
        const char *key,
        archi_rcpointer_t interface,
        archi_context_registry_params_t init_params,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created key is empty");
        return NULL;
    }

    // Prepare the full list of initialization parameters
    struct archi_context_params_info init_params_info;
    {
        ARCHI_ERROR_VAR(error);

        init_params_info = archi_context_params_concatenate(registry,
                init_params.context_key, init_params.list, &error);
        ARCHI_ERROR_ASSIGN(error);

        if (error.code != 0)
            return NULL;
    }

    // Initialize the new context
    archi_context_t context = archi_context_initialize(interface, init_params_info.params,
            ARCHI_ERROR_PARAM);

    archi_context_params_finalize_concatenation(init_params_info);

    if (context == NULL)
        return NULL;

    // Insert the new context into the registry
    archi_context_registry_insert(registry, key, context, ARCHI_ERROR_PARAM);

    // Make the registry the exclusive owner of the new context,
    // or destroy the context in case of insertion failure
    archi_context_finalize(context);

    return context;
}

archi_context_t
archi_context_registry_create_as(
        archi_context_t registry,
        const char *key,
        const char *sample_key,
        archi_context_registry_params_t init_params,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created key is empty");
        return NULL;
    }

    // Obtain the sample context
    archi_context_t sample_context = archi_context_registry_get(
            registry, sample_key, ARCHI_ERROR_PARAM);
    if (sample_context == NULL)
        return NULL;

    // Obtain the interface
    archi_rcpointer_t interface = archi_context_interface(sample_context);

    // Create the new context
    return archi_context_registry_create(registry, key, interface, init_params,
            ARCHI_ERROR_PARAM);
}

archi_context_t
archi_context_registry_create_from(
        archi_context_t registry,
        const char *key,
        const char *source_key,
        archi_context_slot_t source_slot,
        archi_context_registry_params_t init_params,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created key is empty");
        return NULL;
    }

    // Obtain the source context
    archi_context_t source_context = archi_context_registry_get(
            registry, source_key, ARCHI_ERROR_PARAM);
    if (source_context == NULL)
        return NULL;

    // Obtain the interface
    archi_rcpointer_t interface = {0};

    ARCHI_ERROR_VAR(error);

    archi_context_get(source_context, source_slot,
            (archi_context_callback_t){.function = archi_context_callback__getter,
            .data = &interface}, &error);
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        return NULL;

    // Create the new context
    return archi_context_registry_create(registry, key, interface, init_params,
            ARCHI_ERROR_PARAM);
}

void
archi_context_registry_invoke(
        archi_context_t registry,
        const char *key,
        archi_context_slot_t slot,
        archi_context_registry_params_t call_params,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target key is empty");
        return;
    }

    // Obtain the target context
    archi_context_t target_context = archi_context_registry_get(
            registry, key, ARCHI_ERROR_PARAM);
    if (target_context == NULL)
        return;

    // Prepare the full list of call parameters
    struct archi_context_params_info call_params_info;
    {
        ARCHI_ERROR_VAR(error);

        call_params_info = archi_context_params_concatenate(registry,
                call_params.context_key, call_params.list, &error);
        ARCHI_ERROR_ASSIGN(error);

        if (error.code != 0)
            return;
    }

    // Invoke the context call
    archi_context_call(target_context, slot, call_params_info.params,
            (archi_context_callback_t){0}, ARCHI_ERROR_PARAM);

    archi_context_params_finalize_concatenation(call_params_info);
}

void
archi_context_registry_unassign(
        archi_context_t registry,
        const char *key,
        archi_context_slot_t slot,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target key is empty");
        return;
    }

    // Obtain the target context
    archi_context_t target_context = archi_context_registry_get(
            registry, key, ARCHI_ERROR_PARAM);
    if (target_context == NULL)
        return;

    // Unassign the target slot
    archi_context_unset(target_context, slot, ARCHI_ERROR_PARAM);
}

void
archi_context_registry_assign(
        archi_context_t registry,
        const char *key,
        archi_context_slot_t slot,
        bool weak_ref,
        archi_rcpointer_t value,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target key is empty");
        return;
    }

    // Obtain the target context
    archi_context_t target_context = archi_context_registry_get(
            registry, key, ARCHI_ERROR_PARAM);
    if (target_context == NULL)
        return;

    // Assign the value to the target slot
    if (weak_ref)
        value.ref_count = NULL;

    archi_context_set(target_context, slot, value, ARCHI_ERROR_PARAM);
}

void
archi_context_registry_assign_slot(
        archi_context_t registry,
        const char *key,
        archi_context_slot_t slot,
        bool weak_ref,
        const char *source_key,
        archi_context_slot_t source_slot,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target key is empty");
        return;
    }

    // Obtain the target context
    archi_context_t target_context = archi_context_registry_get(
            registry, key, ARCHI_ERROR_PARAM);
    if (target_context == NULL)
        return;

    // Obtain the source context
    archi_context_t source_context = archi_context_registry_get(
            registry, source_key, ARCHI_ERROR_PARAM);
    if (source_context == NULL)
        return;

    // Assign the source slot to the target slot
    archi_context_set_from_get(target_context, slot, source_context, source_slot,
            weak_ref, ARCHI_ERROR_PARAM);
}

void
archi_context_registry_assign_call(
        archi_context_t registry,
        const char *key,
        archi_context_slot_t slot,
        bool weak_ref,
        const char *source_key,
        archi_context_slot_t source_slot,
        archi_context_registry_params_t source_call_params,
        ARCHI_ERROR_PARAM_DECL)
{
    if ((key == NULL) || (key[0] == '\0'))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target key is empty");
        return;
    }

    // Obtain the target context
    archi_context_t target_context = archi_context_registry_get(
            registry, key, ARCHI_ERROR_PARAM);
    if (target_context == NULL)
        return;

    // Obtain the source context
    archi_context_t source_context = archi_context_registry_get(
            registry, source_key, ARCHI_ERROR_PARAM);
    if (source_context == NULL)
        return;

    // Prepare the full list of call parameters
    struct archi_context_params_info call_params_info;
    {
        ARCHI_ERROR_VAR(error);

        call_params_info = archi_context_params_concatenate(registry,
                source_call_params.context_key, source_call_params.list, &error);
        ARCHI_ERROR_ASSIGN(error);

        if (error.code != 0)
            return;
    }

    // Assign the source call to the target slot
    archi_context_set_from_call(target_context, slot, source_context, source_slot,
            call_params_info.params, weak_ref, ARCHI_ERROR_PARAM);

    archi_context_params_finalize_concatenation(call_params_info);
}

