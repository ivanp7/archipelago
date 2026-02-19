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
 * @brief Operations with aggregate objects.
 */

#include "archi/aggr/api/interface.fun.h"
#include "archi/aggr/api/tag.def.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/ref_count.fun.h"

#include <stdlib.h> // for malloc(), free()


struct archi_aggr {
    archi_rcpointer_t interface; ///< Aggregate type interface.
    archi_rcpointer_t metadata; ///< Aggregate type metadata.

    archi_rcpointer_t object; ///< Pointer to aggregate object.

    archi_layout_struct_t layout; ///< Aggregate object layout.
    size_t fam_length; ///< Length of flexible array.

    size_t num_references; ///< Number of references.
    archi_rcpointer_t *reference; ///< Array of references.
};

archi_rcpointer_t
archi_aggr_interface(
        archi_aggr_t aggregate)
{
    if (aggregate == NULL)
        return (archi_rcpointer_t){0};

    return aggregate->interface;
}

archi_rcpointer_t
archi_aggr_metadata(
        archi_aggr_t aggregate)
{
    if (aggregate == NULL)
        return (archi_rcpointer_t){0};

    return aggregate->metadata;
}

archi_rcpointer_t
archi_aggr_object(
        archi_aggr_t aggregate)
{
    if (aggregate == NULL)
        return (archi_rcpointer_t){0};

    return aggregate->object;
}

archi_layout_struct_t
archi_aggr_layout(
        archi_aggr_t aggregate)
{
    if (aggregate == NULL)
        return (archi_layout_struct_t){0};

    return aggregate->layout;
}

size_t
archi_aggr_fam_length(
        archi_aggr_t aggregate)
{
    if (aggregate == NULL)
        return 0;

    return aggregate->fam_length;
}

size_t
archi_aggr_full_size(
        archi_aggr_t aggregate)
{
    if (aggregate == NULL)
        return 0;

    return aggregate->layout.base.size +
        aggregate->layout.fam_stride * aggregate->fam_length;
}

size_t
archi_aggr_num_references(
        archi_aggr_t aggregate)
{
    if (aggregate == NULL)
        return 0;

    return aggregate->num_references;
}

archi_rcpointer_t
archi_aggr_reference(
        archi_aggr_t aggregate,
        size_t index)
{
    if (aggregate == NULL)
        return (archi_rcpointer_t){0};
    else if (index >= aggregate->num_references)
        return (archi_rcpointer_t){0};

    return aggregate->reference[index];
}

/*****************************************************************************/

static
ARCHI_DESTRUCTOR_FUNC(archi_aggr_deallocator)
{
    archi_aggr_t aggregate = data;

    const archi_aggr_interface_t *interface_ptr = aggregate->interface.cptr;

    // Finalize the aggregate object
    if (interface_ptr->final_fn != NULL)
    {
        /*******************************************************************/
        interface_ptr->final_fn(aggregate->object.ptr, aggregate->fam_length,
                aggregate->metadata.cptr);
        /*******************************************************************/
    }

    // Disown the references
    for (size_t i = 0; i < aggregate->num_references; i++)
        archi_rcpointer_disown(aggregate->reference[i]);

    // Dellocate the array of references
    free(aggregate->reference);

    // Deallocate the aggregate object
    free(aggregate->object.ptr);

    // Decrement the reference count of the metadata
    archi_reference_count_decrement(aggregate->metadata.ref_count);

    // Decrement the reference count of the interface
    archi_reference_count_decrement(aggregate->interface.ref_count);

    // Destroy the aggregate object handle
    free(aggregate);
}

archi_aggr_t
archi_aggr_allocate(
        archi_rcpointer_t interface,
        archi_rcpointer_t metadata,

        size_t fam_length,

        ARCHI_ERROR_PARAM_DECL)
{
    // Perform necessary checks
    ARCHI_ERROR_VAR(error);

    if (!archi_pointer_valid(interface.p, &error))
    {
        ARCHI_ERROR_SET(error.code, "aggregate type interface pointer is invalid: %s", error.message);
        return NULL;
    }
    else if (!archi_pointer_valid(metadata.p, &error))
    {
        ARCHI_ERROR_SET(error.code, "aggregate type metadata pointer is invalid: %s", error.message);
        return NULL;
    }

    if (!archi_pointer_attr_compatible(interface.attr,
                archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__AGGR_INTERFACE)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface pointer attributes are incorrect");
        return NULL;
    }
    else if (interface.cptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface pointer is NULL");
        return NULL;
    }
    else if (ARCHI_POINTER_TO_STACK(interface.attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface is on stack");
        return NULL;
    }

    if (ARCHI_POINTER_TO_FUNCTION(metadata.attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type metadata is not data");
        return NULL;
    }
    else if (ARCHI_POINTER_TO_STACK(metadata.attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type metadata is on stack");
        return NULL;
    }

    // Check pointer to interface
    const archi_aggr_interface_t *interface_ptr = interface.cptr;
    if (interface_ptr->layout_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface doesn't have layout_fn()");
        return NULL;
    }
    else if (interface_ptr->numref_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface doesn't have numref_fn()");
        return NULL;
    }
    else if (interface_ptr->init_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface doesn't have init_fn()");
        return NULL;
    }

    // Obtain aggregate type layout
    /*********************************************************************/
    archi_layout_struct_t layout = interface_ptr->layout_fn(metadata.cptr);
    /*********************************************************************/
    if ((layout.fam_stride == 0) && (fam_length != 0))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type doesn't have flexible array member");
        return NULL;
    }

    archi_pointer_attr_t attr;

    if (layout.base.size != 0)
        attr = archi_pointer_attr__pdata(1, layout.base.size, layout.base.alignment, &error);
    else
        attr = archi_pointer_attr__pdata(fam_length, layout.fam_stride, layout.base.alignment, &error);

    if (attr == (archi_pointer_attr_t)-1)
    {
        ARCHI_ERROR_SET(error.code, "aggregate type layout is invalid: %s", error.message);
        return NULL;
    }

    size_t full_size = layout.base.size + layout.fam_stride * fam_length;
    if (full_size == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "full size of aggregate object is zero");
        return NULL;
    }

    // Obtain number of references in the aggregate object
    ARCHI_ERROR_VAR_UNSET(&error);
    /**********************************************************************************/
    size_t num_references = interface_ptr->numref_fn(fam_length, metadata.cptr, &error);
    /**********************************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        return NULL;

    // Allocate the aggregate object handle
    archi_aggr_t aggregate = malloc(sizeof(*aggregate));
    if (aggregate == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate aggregate object handle");
        return NULL;
    }

    *aggregate = (struct archi_aggr){
        .interface = interface,
        .metadata = metadata,

        .object = {
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE | attr,
        },

        .layout = layout,
        .fam_length = fam_length,

        .num_references = num_references,
    };

    // Allocate the aggregate object itself
    aggregate->object.ptr = malloc(full_size);
    if (aggregate->object.ptr == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate aggregate object memory (%zu bytes)",
                full_size);
        goto failure;
    }

    // Allocate the reference counter
    aggregate->object.ref_count =
        archi_reference_count_alloc(archi_aggr_deallocator, aggregate);
    if (aggregate->object.ref_count == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate aggregate object reference counter");
        goto failure;
    }

    // Allocate the array of references
    if (num_references != 0)
    {
        aggregate->reference = malloc(sizeof(*aggregate->reference) * num_references);
        if (aggregate->reference == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array of references of aggregate object (%zu pointers)",
                    num_references);
            goto failure;
        }

        for (size_t i = 0; i < num_references; i++)
            aggregate->reference[i] = (archi_rcpointer_t){0};
    }

    // Initialize the aggregate object
    ARCHI_ERROR_VAR_UNSET(&error);
    /*******************************************************************************/
    interface_ptr->init_fn(aggregate->object.ptr, fam_length, metadata.cptr, &error);
    /*******************************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        goto failure;

    // Increase the reference count of the interface
    archi_reference_count_increment(interface.ref_count);

    // Increase the reference count of the metadata
    archi_reference_count_increment(metadata.ref_count);

    return aggregate;

failure:
    free(aggregate->reference);
    archi_reference_count_free(aggregate->object.ref_count);
    free(aggregate->object.ptr);
    free(aggregate);

    return NULL;
}

void
archi_aggr_free(
        archi_aggr_t aggregate)
{
    if (aggregate == NULL)
        return;

    // Decrement the reference count of the aggregate object
    archi_reference_count_decrement(aggregate->object.ref_count);
}

archi_rcpointer_t
archi_aggr_get(
        archi_aggr_t aggregate,

        const archi_aggr_member_spec_t submember[],
        size_t submember_depth,

        bool immediate_ref,

        ARCHI_ERROR_PARAM_DECL)
{
    // Perform necessary checks
    if (aggregate == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate object is NULL");
        return (archi_rcpointer_t){0};
    }
    else if ((submember == NULL) && (submember_depth != 0))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate object member descriptor is NULL");
        return (archi_rcpointer_t){0};
    }

    for (size_t i = 0; i < submember_depth; i++)
        if (submember[i].name == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate object member name is NULL");
            return (archi_rcpointer_t){0};
        }

    // Process the special case of zero depth
    if (submember_depth == 0)
    {
        ARCHI_ERROR_RESET();
        return archi_aggr_object(aggregate);
    }

    // Check pointer to interface
    const archi_aggr_interface_t *interface_ptr = aggregate->interface.cptr;
    if (interface_ptr->refindex_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface doesn't have refindex_fn()");
        return (archi_rcpointer_t){0};
    }
    else if (interface_ptr->get_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface doesn't have get_fn()");
        return (archi_rcpointer_t){0};
    }

    // Obtain reference index of the member
    ARCHI_ERROR_VAR(error);

    /********************************************************************/
    size_t ref_index = interface_ptr->refindex_fn(aggregate->fam_length,
            submember, submember_depth, aggregate->metadata.cptr, &error);
    /********************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        return (archi_rcpointer_t){0};

    if (!immediate_ref || (ref_index == (size_t)-1)) // value member
    {
        // Call the getter function
        ARCHI_ERROR_VAR_UNSET(&error);
        /*****************************************************************************************/
        archi_pointer_t value = interface_ptr->get_fn(aggregate->object.ptr, aggregate->fam_length,
                submember, submember_depth, aggregate->metadata.cptr, &error);
        /*****************************************************************************************/
        ARCHI_ERROR_ASSIGN(error);

        if (error.code != 0)
            return (archi_rcpointer_t){0};

        if (ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to member is a function");
            return (archi_rcpointer_t){0};
        }
        else if (value.ptr == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to member is NULL");
            return (archi_rcpointer_t){0};
        }
        else if (!archi_pointer_valid(value, &error))
        {
            ARCHI_ERROR_SET(error.code, "pointer to member is invalid: %s", error.message);
            return (archi_rcpointer_t){0};
        }

        return archi_rcpointer(value, aggregate->object.ref_count);
    }
    else // reference member
    {
        // Return the stored reference
        if (ref_index >= aggregate->num_references)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "reference index (%zu) is out of range (%zu)",
                    ref_index, aggregate->num_references);
            return (archi_rcpointer_t){0};
        }

        return aggregate->reference[ref_index];
    }
}

void
archi_aggr_set(
        archi_aggr_t aggregate,

        const archi_aggr_member_spec_t submember[],
        size_t submember_depth,

        archi_rcpointer_t value,

        ARCHI_ERROR_PARAM_DECL)
{
    // Perform necessary checks
    if (aggregate == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate object is NULL");
        return;
    }
    else if (submember == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate object member descriptor is NULL");
        return;
    }
    else if (submember_depth == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate object member depth is zero");
        return;
    }

    for (size_t i = 0; i < submember_depth; i++)
        if (submember[i].name == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate object member name is NULL");
            return;
        }

    ARCHI_ERROR_VAR(error);

    if (!archi_pointer_valid(value.p, &error))
    {
        ARCHI_ERROR_SET(error.code, "value set to aggregate object member is invalid: %s", error.message);
        return;
    }

    // Reset reference counter for a null value
    if (ARCHI_POINTER_IS_NULL(value.p))
        value.ref_count = NULL;

    // Check pointer to interface
    const archi_aggr_interface_t *interface_ptr = aggregate->interface.cptr;
    if (interface_ptr->refindex_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface doesn't have refindex_fn()");
        return;
    }
    else if (interface_ptr->set_fn == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "aggregate type interface doesn't have set_fn()");
        return;
    }

    // Obtain reference index of the member
    ARCHI_ERROR_VAR_UNSET(&error);
    /********************************************************************/
    size_t ref_index = interface_ptr->refindex_fn(aggregate->fam_length,
            submember, submember_depth, aggregate->metadata.cptr, &error);
    /********************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
        return;

    if ((ref_index != (size_t)-1) && (ref_index >= aggregate->num_references))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "reference index (%zu) is out of range (%zu)",
                ref_index, aggregate->num_references);
        return;
    }

    // Own the assigned value
    value = archi_rcpointer_own(value, ARCHI_ERROR_PARAM);
    if (!value.attr)
        return;

    // Call the setter function
    ARCHI_ERROR_VAR_UNSET(&error);
    /***************************************************************************/
    interface_ptr->set_fn(aggregate->object.ptr, aggregate->fam_length,
            submember, submember_depth, value, aggregate->metadata.cptr, &error);
    /***************************************************************************/
    ARCHI_ERROR_ASSIGN(error);

    if (error.code != 0)
    {
        archi_rcpointer_disown(value);
        return;
    }

    if (ref_index != (size_t)-1)
    {
        // Update the reference
        archi_rcpointer_disown(aggregate->reference[ref_index]);
        aggregate->reference[ref_index] = value;
    }
}

