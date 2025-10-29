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
 * @brief Interface operations on contexts.
 */

#include "archi/context/api/interface.fun.h"
#include "archi/context/api/interface.typ.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/base/ref_count.fun.h"

#include <stdlib.h> // for malloc(), free()

struct archi_context {
    archi_rcpointer_t interface; ///< Context interface.
    archi_rcpointer_t *data;     ///< Context data.
    archi_reference_count_t ref_count; ///< Context reference counter.
};

archi_rcpointer_t
archi_context_interface(
        archi_context_t context)
{
    if (context == NULL)
        return (archi_rcpointer_t){0};

    return context->interface;
}

archi_rcpointer_t
archi_context_data(
        archi_context_t context)
{
    if (context == NULL)
        return (archi_rcpointer_t){0};

    archi_rcpointer_t context_data = *context->data;
    context_data.ref_count = context->ref_count;

    return context_data;
}

/*****************************************************************************/

static
ARCHI_DESTRUCTOR_FUNC(archi_context_destructor)
{
    archi_context_t context = data;

    const archi_context_interface_t *interface_ptr = context->interface.cptr;

    // Finalize the context
    if (interface_ptr->final_fn != NULL)
    {
        /*************************************/
        interface_ptr->final_fn(context->data);
        /*************************************/
    }

    // Decrement the reference count of the interface
    archi_reference_count_decrement(context->interface.ref_count);

    // Destroy the context object
    free(context);
}

archi_context_t
archi_context_initialize(
        archi_rcpointer_t interface,
        const archi_kvlist_rc_t *params,

        ARCHI_ERROR_PARAMETER_DECL)
{
    // Perform necessary checks
    archi_error_t error;
    if (!archi_pointer_valid(interface.p, &error))
    {
        ARCHI_ERROR_SET(error.code, "context interface pointer is invalid: %s", error.message);
        return NULL;
    }

    if (!archi_pointer_attr_compatible(interface.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_interface_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context interface pointer attributes are incorrect");
        return NULL;
    }
    else if (ARCHI_POINTER_TO_STACK(interface.attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context interface is on stack");
        return NULL;
    }

    const archi_context_interface_t *interface_ptr = interface.ptr;
    if (interface_ptr->init_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context interface doesn't have init_fn()");
        return NULL;
    }

    // Allocate the context object and its reference counter
    archi_context_t context = malloc(sizeof(*context));
    if (context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context object");
        return NULL;
    }

    *context = (struct archi_context){
        .interface = interface,
        .ref_count = archi_reference_count_alloc(archi_context_destructor, context),
    };

    if (context->ref_count == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context refcounter");
        goto failure;
    }

    // Initialize the context
    ARCHI_ERROR_RESET_VAR(&error);
    /*****************************************************/
    context->data = interface_ptr->init_fn(params, &error);
    /*****************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (context->data == NULL)
    {
        if (error.code == 0)
            ARCHI_ERROR_SET(ARCHI__ECONTRACT, "init_fn() returned zero status code on failure");
        goto failure;
    }

    // Increment the reference count of the interface
    archi_reference_count_increment(interface.ref_count);

    return context;

failure:
    archi_reference_count_free(context->ref_count);
    free(context);

    return NULL;
}

void
archi_context_finalize(
        archi_context_t context)
{
    if (context == NULL)
        return;

    // Decrement the reference count of the context itself
    archi_reference_count_decrement(context->ref_count);
}

struct archi_context_callback_wrapper_data {
    archi_context_callback_t callback;
    bool called_once;

    archi_reference_count_t context_ref_count;
    archi_reference_count_t interface_ref_count;
};

static
ARCHI_CONTEXT_CALLBACK_FUNC(archi_context_callback_wrapper)
{
    struct archi_context_callback_wrapper_data *wrapper_data = data;

    // Protect the callback from multiple calls
    if (!wrapper_data->called_once)
        wrapper_data->called_once = true;
    else
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context callback has been called more than once");
        return;
    }

    // Check the pointer for validness
    {
        archi_error_t error;

        if (!archi_pointer_valid(value.p, &error))
        {
            ARCHI_ERROR_SET(error.code, "output pointer is invalid: %s", error.message);
            return;
        }
        else if (ARCHI_POINTER_TO_STACK(value.attr) && (value.ref_count != NULL))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "output pointer to stack has reference counter");
            return;
        }
    }

    // Substitute reference counter if needed
    if (value.ref_count == ARCHI_CONTEXT_REF_COUNT)
    {
        if (ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "output pointer to function wants context reference counter");
            return;
        }

        value.ref_count = wrapper_data->context_ref_count;
    }
    else if (value.ref_count == ARCHI_CONTEXT_INTERFACE_REF_COUNT)
        value.ref_count = wrapper_data->interface_ref_count;

    // Call the user callback
    /*********************************************************************/
    if (wrapper_data->callback.function != NULL)
        wrapper_data->callback.function(value, wrapper_data->callback.data,
                ARCHI_ERROR_PARAMETER);
    /*********************************************************************/
}

void
archi_context_get(
        archi_context_t context,
        archi_context_slot_t slot,
        archi_context_callback_t callback,

        ARCHI_ERROR_PARAMETER_DECL)
{
    // Perform necessary checks
    if (context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context is NULL");
        return;
    }
    else if (callback.function == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context callback function is NULL");
        return;
    }

    if (slot.name == NULL)
        slot.name = "";

    if (!ARCHI_CONTEXT_SLOT_VALID(slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context slot is invalid");
        return;
    }

    // Process empty slot
    if (ARCHI_CONTEXT_SLOT_EMPTY(slot))
    {
        /***********************************************************/
        callback.function(archi_context_data(context), callback.data,
                ARCHI_ERROR_PARAMETER);
        /***********************************************************/
        return;
    }

    // Check context interface
    const archi_context_interface_t *interface_ptr = context->interface.cptr;
    if (interface_ptr->eval_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context interface doesn't have eval_fn()");
        return;
    }

    // Prepare callback wrapper data
    struct archi_context_callback_wrapper_data wrapper_data = {
        .callback = callback,

        .context_ref_count = context->ref_count,
        .interface_ref_count = context->interface.ref_count,
    };

    // Call the evaluation function
    archi_error_t error = {0};
    /****************************************************************************/
    interface_ptr->eval_fn(context->data, slot, false, NULL,
            (archi_context_callback_t){.function = archi_context_callback_wrapper,
            .data = &wrapper_data}, &error);
    /****************************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (!wrapper_data.called_once)
    {
        if (error.code == 0)
            ARCHI_ERROR_SET(ARCHI__ECONTRACT, "eval_fn() didn't call callback function and returned zero status code");
    }
}

void
archi_context_call(
        archi_context_t context,
        archi_context_slot_t slot,
        const archi_kvlist_rc_t *params,
        archi_context_callback_t callback,

        ARCHI_ERROR_PARAMETER_DECL)
{
    // Perform necessary checks
    if (context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context is NULL");
        return;
    }

    if (slot.name == NULL)
        slot.name = "";

    if (!ARCHI_CONTEXT_SLOT_VALID(slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context slot is invalid");
        return;
    }

    // Check context interface
    const archi_context_interface_t *interface_ptr = context->interface.cptr;
    if (interface_ptr->eval_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context interface doesn't have eval_fn()");
        return;
    }

    // Prepare callback wrapper data
    struct archi_context_callback_wrapper_data wrapper_data = {
        .callback = callback,

        .context_ref_count = context->ref_count,
        .interface_ref_count = context->interface.ref_count,
    };

    // Call the evaluation function
    archi_error_t error = {0};
    /****************************************************************************/
    interface_ptr->eval_fn(context->data, slot, true, params,
            (archi_context_callback_t){.function = archi_context_callback_wrapper,
            .data = &wrapper_data}, &error);
    /****************************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (!wrapper_data.called_once)
    {
        if ((error.code == 0) && (callback.function != NULL))
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "eval_fn() didn't call callback function, but an output value was expected");
    }
}

void
archi_context_set(
        archi_context_t context,
        archi_context_slot_t slot,
        archi_rcpointer_t value,

        ARCHI_ERROR_PARAMETER_DECL)
{
    // Perform necessary checks
    if (context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context is NULL");
        return;
    }

    if (slot.name == NULL)
        slot.name = "";

    if (!ARCHI_CONTEXT_SLOT_VALID(slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context slot is invalid");
        return;
    }
    else if (ARCHI_CONTEXT_SLOT_EMPTY(slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context slot is empty");
        return;
    }

    archi_error_t error;
    if (!archi_pointer_valid(value.p, &error))
    {
        ARCHI_ERROR_SET(error.code, "value assigned to context slot is invalid: %s", error.message);
        return;
    }

    // Check context interface
    const archi_context_interface_t *interface_ptr = context->interface.cptr;
    if (interface_ptr->set_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context interface doesn't have set_fn()");
        return;
    }

    // Call the setter function
    /***********************************************************************/
    interface_ptr->set_fn(context->data, slot, value, ARCHI_ERROR_PARAMETER);
    /***********************************************************************/
}

/*****************************************************************************/

struct archi_context_set__callback_data {
    archi_context_set_func_t set_fn;
    archi_rcpointer_t *context_data;
    archi_context_slot_t slot;
    bool no_refcount;
};

static
ARCHI_CONTEXT_CALLBACK_FUNC(archi_context_set__callback)
{
    struct archi_context_set__callback_data *callback_data = data;

    if (callback_data->no_refcount)
        value.ref_count = NULL;

    /*********************************************************/
    callback_data->set_fn(callback_data->context_data,
            callback_data->slot, value, ARCHI_ERROR_PARAMETER);
    /*********************************************************/
}

void
archi_context_set_from_get(
        archi_context_t context,
        archi_context_slot_t slot,

        archi_context_t src_context,
        archi_context_slot_t src_slot,
        bool src_no_refcount,

        ARCHI_ERROR_PARAMETER_DECL)
{
    // Perform necessary checks
    if (context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination context is NULL");
        return;
    }
    else if (src_context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context is NULL");
        return;
    }

    if (slot.name == NULL)
        slot.name = "";
    if (src_slot.name == NULL)
        src_slot.name = "";

    if (!ARCHI_CONTEXT_SLOT_VALID(slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination context slot is invalid");
        return;
    }
    else if (!ARCHI_CONTEXT_SLOT_VALID(src_slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context slot is invalid");
        return;
    }
    else if (ARCHI_CONTEXT_SLOT_EMPTY(slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination context slot is empty");
        return;
    }

    // Check context interfaces
    const archi_context_interface_t *interface_ptr = context->interface.cptr;
    const archi_context_interface_t *src_interface_ptr = src_context->interface.cptr;

    if (interface_ptr->set_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination context interface doesn't have set_fn()");
        return;
    }
    else if (!ARCHI_CONTEXT_SLOT_EMPTY(src_slot) && (src_interface_ptr->eval_fn == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context interface doesn't have eval_fn()");
        return;
    }

    // Process empty source slot
    if (ARCHI_CONTEXT_SLOT_EMPTY(src_slot))
    {
        /**************************************************************/
        interface_ptr->set_fn(context->data, slot,
                archi_context_data(src_context), ARCHI_ERROR_PARAMETER);
        /**************************************************************/
        return;
    }

    // Prepare callback data
    struct archi_context_set__callback_data callback_data = {
        .set_fn = interface_ptr->set_fn,
        .context_data = context->data,
        .slot = slot,
        .no_refcount = src_no_refcount,
    };

    // Prepare callback wrapper data
    struct archi_context_callback_wrapper_data wrapper_data = {
        .callback = {
            .function = archi_context_set__callback,
            .data = &callback_data,
        },

        .context_ref_count = src_context->ref_count,
        .interface_ref_count = src_context->interface.ref_count,
    };

    // Call the evaluation function
    archi_error_t error = {0};
    /****************************************************************************/
    src_interface_ptr->eval_fn(src_context->data, src_slot, false, NULL,
            (archi_context_callback_t){.function = archi_context_callback_wrapper,
            .data = &wrapper_data}, &error);
    /****************************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (!wrapper_data.called_once)
    {
        if (error.code == 0)
            ARCHI_ERROR_SET(ARCHI__ECONTRACT, "eval_fn() didn't call callback function and returned zero status code");
    }
}

void
archi_context_set_from_call(
        archi_context_t context,
        archi_context_slot_t slot,

        archi_context_t src_context,
        archi_context_slot_t src_slot,
        const archi_kvlist_rc_t *src_params,
        bool src_no_refcount,

        ARCHI_ERROR_PARAMETER_DECL)
{
    // Perform necessary checks
    if (context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination context is NULL");
        return;
    }
    else if (src_context == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context is NULL");
        return;
    }

    if (slot.name == NULL)
        slot.name = "";
    if (src_slot.name == NULL)
        src_slot.name = "";

    if (!ARCHI_CONTEXT_SLOT_VALID(slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination context slot is invalid");
        return;
    }
    else if (!ARCHI_CONTEXT_SLOT_VALID(src_slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context slot is invalid");
        return;
    }
    else if (ARCHI_CONTEXT_SLOT_EMPTY(slot))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination context slot is empty");
        return;
    }

    // Check context interfaces
    const archi_context_interface_t *interface_ptr = context->interface.cptr;
    const archi_context_interface_t *src_interface_ptr = src_context->interface.cptr;

    if (interface_ptr->set_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination context interface doesn't have set_fn()");
        return;
    }
    else if (src_interface_ptr->eval_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context interface doesn't have eval_fn()");
        return;
    }

    // Prepare callback data
    struct archi_context_set__callback_data callback_data = {
        .set_fn = interface_ptr->set_fn,
        .context_data = context->data,
        .slot = slot,
        .no_refcount = src_no_refcount,
    };

    // Prepare callback wrapper data
    struct archi_context_callback_wrapper_data wrapper_data = {
        .callback = {
            .function = archi_context_set__callback,
            .data = &callback_data,
        },

        .context_ref_count = src_context->ref_count,
        .interface_ref_count = src_context->interface.ref_count,
    };

    // Call the evaluation function
    archi_error_t error = {0};
    /****************************************************************************/
    src_interface_ptr->eval_fn(src_context->data, src_slot, true, src_params,
            (archi_context_callback_t){.function = archi_context_callback_wrapper,
            .data = &wrapper_data}, &error);
    /****************************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (!wrapper_data.called_once)
    {
        if (error.code == 0)
            ARCHI_ERROR_SET(ARCHI__ECONTRACT, "eval_fn() didn't call callback function and returned zero status code");
    }
}

