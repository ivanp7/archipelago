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
 * @brief Interface operations on contexts.
 */

#include "archi/context/api/interface.fun.h"
#include "archi/context/api/interface.typ.h"
#include "archipelago/base/ref_count.fun.h"

#include <stdlib.h> // for malloc(), free()

struct archi_context {
    archi_pointer_t interface; ///< Context interface.
    archi_pointer_t *data;     ///< Context data.
    archi_reference_count_t ref_count; ///< Original reference count returned by init_fn().
};

archi_pointer_t
archi_context_interface(
        archi_context_t context)
{
    if (context == NULL)
        return (archi_pointer_t){0};

    return context->interface;
}

archi_pointer_t
archi_context_data(
        archi_context_t context)
{
    if (context == NULL)
        return (archi_pointer_t){0};

    return *context->data;
}

/*****************************************************************************/

static
ARCHI_DESTRUCTOR_FUNC(archi_context_destructor)
{
    archi_context_t context = data;

    const archi_context_interface_t *interface_ptr = context->interface.ptr;

    // Restore the original reference count
    context->data->ref_count = context->ref_count;

    // Finalize the context
    /*****************************************/
    if (interface_ptr->final_fn != NULL)
        interface_ptr->final_fn(context->data);
    /*****************************************/

    // Decrement the reference count of the interface
    archi_reference_count_decrement(context->interface.ref_count);

    // Destroy the context object
    free(context);
}

archi_context_t
archi_context_initialize(
        archi_pointer_t interface,
        const archi_named_pointer_list_t *params,
        archi_status_t *code)
{
    if ((interface.flags & ARCHI_POINTER_FLAG_FUNCTION) || (interface.ptr == NULL))
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EMISUSE;

        return NULL;
    }

    const archi_context_interface_t *interface_ptr = interface.ptr;
    if (interface_ptr->init_fn == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EINTERFACE;

        return NULL;
    }

    // Allocate the context object
    archi_context_t context = malloc(sizeof(*context));
    if (context == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        return NULL;
    }

    *context = (struct archi_context){
        .interface = interface,
    };

    // Allocate the reference counter
    archi_reference_count_t ref_count =
        archi_reference_count_alloc(archi_context_destructor, context);
    if (ref_count == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_ENOMEMORY;

        free(context);
        return NULL;
    }

    // Initialize the context
    /************************************************************************/
    archi_status_t code_init = interface_ptr->init_fn(&context->data, params);
    /************************************************************************/

    if (code_init < 0)
    {
        if (code != NULL)
            *code = code_init;

        archi_reference_count_free(ref_count);
        free(context);
        return NULL;
    }
    else if (context->data == NULL)
    {
        if (code != NULL)
            *code = ARCHI_STATUS_EFAILURE;

        archi_reference_count_free(ref_count);
        free(context);
        return NULL;
    }

    // Save the original reference counter to the context
    context->ref_count = context->data->ref_count;

    // Assign the new reference counter to the context data
    context->data->ref_count = ref_count;

    // Increment the reference count of the interface
    archi_reference_count_increment(interface.ref_count);

    if (code != NULL)
        *code = code_init;

    return context;
}

void
archi_context_finalize(
        archi_context_t context)
{
    if (context == NULL)
        return;

    // Decrement the reference count
    archi_reference_count_decrement(context->data->ref_count);
}

/*****************************************************************************/

archi_pointer_t
archi_context_get_slot(
        archi_context_t context,
        archi_context_slot_t slot,
        archi_status_t *code)
{
    archi_status_t code_get;
    archi_pointer_t value = {0};

    if (context == NULL)
    {
        code_get = ARCHI_STATUS_EMISUSE;
        goto finish;
    }
    else if ((slot.name == NULL) || ((slot.num_indices > 0) && (slot.index == NULL)))
    {
        code_get = ARCHI_STATUS_EMISUSE;
        goto finish;
    }

    if ((slot.name[0] == '\0') && (slot.num_indices == 0))
    {
        value = archi_context_data(context);
        code_get = 0;
    }
    else
    {
        const archi_context_interface_t *interface_ptr = context->interface.ptr;
        if (interface_ptr->get_fn == NULL)
        {
            code_get = ARCHI_STATUS_EINTERFACE;
            goto finish;
        }

        /************************************************************/
        code_get = interface_ptr->get_fn(context->data, slot, &value);
        /************************************************************/
    }

finish:
    if (code != NULL)
        *code = code_get;

    return value;
}

archi_status_t
archi_context_set_slot(
        archi_context_t context,
        archi_context_slot_t slot,
        archi_pointer_t value)
{
    if (context == NULL)
        return ARCHI_STATUS_EMISUSE;
    else if ((slot.name == NULL) || ((slot.num_indices > 0) && (slot.index == NULL)))
        return ARCHI_STATUS_EMISUSE;

    if ((slot.name[0] == '\0') && (slot.num_indices == 0))
        return ARCHI_STATUS_EMISUSE;

    const archi_context_interface_t *interface_ptr = context->interface.ptr;
    if (interface_ptr->set_fn == NULL)
        return ARCHI_STATUS_EINTERFACE;

    /*******************************************************/
    return interface_ptr->set_fn(context->data, slot, value);
    /*******************************************************/
}

archi_status_t
archi_context_copy_slot(
        archi_context_t context,
        archi_context_slot_t slot,

        archi_context_t src_context,
        archi_context_slot_t src_slot)
{
    if ((context == NULL) || (src_context == NULL))
        return ARCHI_STATUS_EMISUSE;
    else if ((slot.name == NULL) || ((slot.num_indices > 0) && (slot.index == NULL)))
        return ARCHI_STATUS_EMISUSE;
    else if ((src_slot.name == NULL) || ((src_slot.num_indices > 0) && (src_slot.index == NULL)))
        return ARCHI_STATUS_EMISUSE;

    const archi_context_interface_t *interface_ptr = context->interface.ptr;
    const archi_context_interface_t *src_interface_ptr = src_context->interface.ptr;
    if ((interface_ptr->set_fn == NULL) || (src_interface_ptr->get_fn == NULL))
        return ARCHI_STATUS_EINTERFACE;

    archi_pointer_t value = {0};

    /***********************************************************************************/
    archi_status_t code = src_interface_ptr->get_fn(src_context->data, src_slot, &value);
    /***********************************************************************************/
    if (code != 0)
        return code;

    /*******************************************************/
    return interface_ptr->set_fn(context->data, slot, value);
    /*******************************************************/
}

/*****************************************************************************/

archi_status_t
archi_context_act(
        archi_context_t context,
        archi_context_slot_t action,
        const archi_named_pointer_list_t *params)
{
    if (context == NULL)
        return ARCHI_STATUS_EMISUSE;
    else if ((action.name == NULL) || ((action.num_indices > 0) && (action.index == NULL)))
        return ARCHI_STATUS_EMISUSE;

    const archi_context_interface_t *interface_ptr = context->interface.ptr;
    if (interface_ptr->act_fn == NULL)
        return ARCHI_STATUS_EINTERFACE;

    /**********************************************************/
    return interface_ptr->act_fn(context->data, action, params);
    /**********************************************************/
}

