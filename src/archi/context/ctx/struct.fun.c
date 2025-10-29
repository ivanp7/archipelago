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
 * @brief Template of application context interface for simple C structures.
 */

#include "archi/context/ctx/struct.fun.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/util/struct.typ.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/base/ref_count.fun.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"
#include "archipelago/util/string.typ.h"

#include <stdlib.h> // for malloc(), aligned_alloc(), free()
#include <string.h> // for memcpy(), memmove()
#include <ctype.h> // for isalnum()
#include <assert.h>

struct archi_context_data__struct {
    archi_rcpointer_t structure;  ///< The structure object itself.
    archi_rcpointer_t *reference; ///< Array of references to pointees.

    const archi_struct_info_t *struct_info; ///< Description of the structure type.
};

static
ARCHI_STRING_COMPARISON_FUNC(archi_context_struct_strcmp)
{
    const char *param_name = lhs;
    const char *field_name = rhs;

    for (;;)
    {
        if ((*param_name == '\0') && (*field_name == '\0'))
            return 0;
        else if ((*param_name == *field_name) ||
                ((*param_name == '_') && (*field_name == '.')))
        {
            param_name++;
            field_name++;
        }
        else
            return 1;
    }
}

static
bool
archi_context_struct_assign_vfield(
        char *structure,
        archi_struct_field_info_t vfield,
        archi_rcpointer_t value,
        ARCHI_ERROR_PARAMETER_DECL)
{
    if (!archi_pointer_attr_compatible(value.attr,
                archi_pointer_attr__transp_data(vfield.length, vfield.stride, vfield.alignment, NULL)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value type is not compatible with field '%s'",
                vfield.name);
        return false;
    }

    memmove(structure + vfield.offset, value.ptr, vfield.length * vfield.stride);

    ARCHI_ERROR_RESET();
    return true;
}

static
bool
archi_context_struct_assign_pointer(
        char *structure,
        archi_rcpointer_t *reference,
        archi_struct_field_info_t pfield,
        archi_rcpointer_t pointer,
        ARCHI_ERROR_PARAMETER_DECL)
{
    archi_pointer_attr_t pfield_attr;

    if (pfield.type == ARCHI_STRUCT_FIELD_TYPE__TRANSP_DATA)
        pfield_attr = archi_pointer_attr__transp_data(pfield.length, pfield.stride, pfield.alignment, NULL);
    else if (pfield.type == ARCHI_STRUCT_FIELD_TYPE__DATA)
        pfield_attr = archi_pointer_attr__opaque_data(pfield.tag);
    else
        pfield_attr = archi_pointer_attr__function(pfield.tag);

    if (!archi_pointer_attr_compatible(pointer.attr, pfield_attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned pointer type is not compatible with field '%s'",
                pfield.name);
        return false;
    }

    pointer = archi_rcpointer_own_disown(pointer, *reference, ARCHI_ERROR_PARAMETER);
    if (!pointer.attr) // failed to own
        return false;

    *reference = pointer;

    if (!ARCHI_POINTER_TO_FUNCTION(pointer.attr))
        memcpy(structure + pfield.offset, &pointer.ptr, sizeof(pointer.ptr));
    else
        memcpy(structure + pfield.offset, &pointer.fptr, sizeof(pointer.fptr));

    ARCHI_ERROR_RESET();
    return true;
}

archi_rcpointer_t*
archi_context_init__struct(
        const archi_kvlist_rc_t *params,
        ARCHI_ERROR_PARAMETER_DECL,

        const archi_struct_info_t *struct_info)
{
    // Assert correctness of structure description
    assert(struct_info != NULL);
    assert((struct_info->num_vfields == 0) || (struct_info->vfield != NULL));
    assert((struct_info->num_pfields == 0) || (struct_info->pfield != NULL));
    assert(struct_info->size != 0);
    assert(struct_info->size <= ARCHI_POINTER_DATA_STRIDE_MAX);
    assert(struct_info->alignment != 0);
    assert((struct_info->alignment & (struct_info->alignment - 1)) == 0);
    assert(struct_info->alignment <= ARCHI_POINTER_DATA_ALIGNMENT_MAX);
    assert(struct_info->size % struct_info->alignment == 0);
    assert(struct_info->initializer != NULL);

#ifndef NDEBUG
    // Assert correctness of value field descriptions
    for (size_t i = 0; i < struct_info->num_vfields; i++)
    {
        archi_struct_field_info_t vfield = struct_info->vfield[i];

        assert(vfield.name != NULL);
        assert(vfield.name[0] != '\0');
        assert(vfield.type == ARCHI_STRUCT_FIELD_TYPE__TRANSP_DATA);
        assert(archi_pointer_attr__transp_data(vfield.length, vfield.stride, vfield.alignment, NULL) !=
                (archi_pointer_attr_t)-1);
        assert(vfield.length > 0);
        assert(vfield.offset + vfield.length * vfield.stride <= struct_info->size);
        assert(vfield.offset % vfield.alignment == 0);
        assert(vfield.alignment <= struct_info->alignment);
    }

    // Assert correctness of pointer field descriptions
    for (size_t i = 0; i < struct_info->num_pfields; i++)
    {
        archi_struct_field_info_t pfield = struct_info->pfield[i];

        assert(pfield.name != NULL);
        assert(pfield.name[0] != '\0');
        assert((pfield.type >= ARCHI_STRUCT_FIELD_TYPE__DATA) &&
                (pfield.type <= ARCHI_STRUCT_FIELD_TYPE__FUNCTION));

        if (pfield.type == ARCHI_STRUCT_FIELD_TYPE__DATA)
        {
            assert(archi_pointer_attr__opaque_data(pfield.tag) != (archi_pointer_attr_t)-1);
            assert(pfield.offset + sizeof(archi_data_t) <= struct_info->size);
            assert(pfield.offset % alignof(archi_data_t) == 0);
            assert(alignof(archi_data_t) <= struct_info->alignment);
        }
        else if (pfield.type == ARCHI_STRUCT_FIELD_TYPE__TRANSP_DATA)
        {
            assert(archi_pointer_attr__transp_data(pfield.length, pfield.stride, pfield.alignment, NULL) !=
                    (archi_pointer_attr_t)-1);
            assert(pfield.offset + sizeof(archi_data_t) <= struct_info->size);
            assert(pfield.offset % alignof(archi_data_t) == 0);
            assert(alignof(archi_data_t) <= struct_info->alignment);
        }
        else if (pfield.type == ARCHI_STRUCT_FIELD_TYPE__FUNCTION)
        {
            assert(archi_pointer_attr__function(pfield.tag) != 0);
            assert(pfield.offset + sizeof(archi_function_t) <= struct_info->size);
            assert(pfield.offset % alignof(archi_function_t) == 0);
            assert(alignof(archi_function_t) <= struct_info->alignment);
        }
    }
#endif

    // Allocate the context data and the structure object
    struct archi_context_data__struct *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    *context_data = (struct archi_context_data__struct){
        .structure = {
            .ptr = aligned_alloc(struct_info->alignment, struct_info->size),
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__transp_data(1, struct_info->size, struct_info->alignment, NULL),
        },
        .struct_info = struct_info,
    };

    if (context_data->structure.ptr == NULL)
    {
        free(context_data);

        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate memory for structure object (%zu bytes, alignment = %#zx)",
                struct_info->size, struct_info->alignment);
        return NULL;
    }

    // Initialize the structure object
    memcpy(context_data->structure.ptr, struct_info->initializer, struct_info->size);

    // Allocate and initialize array of references
    if (struct_info->num_pfields != 0)
    {
        context_data->reference = malloc(sizeof(*context_data->reference) * struct_info->num_pfields);
        if (context_data->reference == NULL)
        {
            free(context_data->structure.ptr);
            free(context_data);

            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array [%zu] of references to pointer fields",
                    struct_info->num_pfields);
            return NULL;
        }

        for (size_t i = 0; i < struct_info->num_pfields; i++)
            context_data->reference[i] = (archi_rcpointer_t){0};
    }

    // Parse parameters
    if (params != NULL)
    {
        size_t num_parameters = struct_info->num_vfields + struct_info->num_pfields;

        // Allocate array of parameter descriptions
        archi_kvlist_parameter_t *parsed = malloc(sizeof(*parsed) * num_parameters);
        if (parsed == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate array [%zu] of context parameter descriptions",
                    num_parameters);
            goto failure;
        }

        // Initialize parameter descriptions
        for (size_t i = 0; i < num_parameters; i++)
        {
            if (i < struct_info->num_vfields)
            {
                archi_struct_field_info_t vfield = struct_info->vfield[i];

                parsed[i] = (archi_kvlist_parameter_t){
                    .name = vfield.name,
                    .value.attr = archi_pointer_attr__transp_data(
                            vfield.length, vfield.stride, vfield.alignment, NULL),
                };
            }
            else
            {
                archi_struct_field_info_t pfield = struct_info->pfield[i - struct_info->num_vfields];

                parsed[i] = (archi_kvlist_parameter_t){
                    .name = pfield.name,
                };

                switch (pfield.type)
                {
                    case ARCHI_STRUCT_FIELD_TYPE__DATA:
                        parsed[i].value.attr = archi_pointer_attr__opaque_data(pfield.tag);
                        break;

                    case ARCHI_STRUCT_FIELD_TYPE__TRANSP_DATA:
                        parsed[i].value.attr = archi_pointer_attr__transp_data(
                                pfield.length, pfield.stride, pfield.alignment, NULL);
                        break;

                    case ARCHI_STRUCT_FIELD_TYPE__FUNCTION:
                        parsed[i].value.attr = archi_pointer_attr__function(pfield.tag);
                        break;
                }
            }
        }

        // Parse parameter list
        if (!archi_kvlist_parameters_parse(params, parsed, num_parameters, false,
                    archi_context_struct_strcmp, ARCHI_ERROR_PARAMETER))
        {
            free(parsed);
            goto failure;
        }

        // Assign values
        for (size_t i = 0; i < struct_info->num_vfields; i++)
        {
            if (!parsed[i].value_set)
                continue;

            if (!archi_context_struct_assign_vfield(context_data->structure.ptr,
                        struct_info->vfield[i], parsed[i].value, ARCHI_ERROR_PARAMETER))
            {
                free(parsed);
                goto failure;
            }
        }

        // Assign pointers
        for (size_t i = 0; i < struct_info->num_pfields; i++)
        {
            archi_kvlist_parameter_t param = parsed[i + struct_info->num_vfields];
            if (!param.value_set)
                continue;

            if (!archi_context_struct_assign_pointer(context_data->structure.ptr,
                        &context_data->reference[i], struct_info->pfield[i], param.value,
                        ARCHI_ERROR_PARAMETER))
            {
                free(parsed);
                goto failure;
            }
        }

        free(parsed);
    }

    ARCHI_ERROR_RESET();
    return (archi_rcpointer_t*)context_data;

failure:
    for (size_t i = 0; i < struct_info->num_pfields; i++)
        archi_rcpointer_disown(context_data->reference[i]);

    free(context_data->reference);
    free(context_data->structure.ptr);
    free(context_data);

    return NULL;
}

ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__struct)
{
    struct archi_context_data__struct *context_data =
        (struct archi_context_data__struct*)context;

    const archi_struct_info_t *struct_info = context_data->struct_info;

    // Decrement reference counters
    for (size_t i = 0; i < struct_info->num_pfields; i++)
        archi_rcpointer_disown(context_data->reference[i]);

    // Free the memory
    free(context_data->reference);
    free(context_data->structure.ptr);
    free(context_data);
}

ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__struct)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (slot.num_indices != 0)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
        return;
    }

    struct archi_context_data__struct *context_data =
        (struct archi_context_data__struct*)context;

    const archi_struct_info_t *struct_info = context_data->struct_info;

    // Check value fields
    for (size_t i = 0; i < struct_info->num_vfields; i++)
    {
        archi_struct_field_info_t vfield = struct_info->vfield[i];

        if (ARCHI_STRING_COMPARE(vfield.name, !=, slot.name))
            continue;

        archi_rcpointer_t value = {
            .ptr = (char*)context_data->structure.ptr + vfield.offset,
            .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
                archi_pointer_attr__transp_data(vfield.length, vfield.stride, vfield.alignment, NULL),
            .ref_count = ARCHI_CONTEXT_REF_COUNT,
        };

        ARCHI_CONTEXT_YIELD(value);
        return;
    }

    // Check pointer fields
    for (size_t i = 0; i < struct_info->num_pfields; i++)
    {
        archi_struct_field_info_t pfield = struct_info->pfield[i];

        if (ARCHI_STRING_COMPARE(pfield.name, !=, slot.name))
            continue;

        ARCHI_CONTEXT_YIELD(context_data->reference[i]);
        return;
    }

    ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

ARCHI_CONTEXT_SET_FUNC(archi_context_set__struct)
{
    if (slot.num_indices != 0)
    {
        ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
        return;
    }

    struct archi_context_data__struct *context_data =
        (struct archi_context_data__struct*)context;

    const archi_struct_info_t *struct_info = context_data->struct_info;

    // Check value fields
    for (size_t i = 0; i < struct_info->num_vfields; i++)
    {
        archi_struct_field_info_t vfield = struct_info->vfield[i];

        if (ARCHI_STRING_COMPARE(vfield.name, !=, slot.name))
            continue;

        archi_context_struct_assign_vfield(context_data->structure.ptr, vfield, value,
                ARCHI_ERROR_PARAMETER);

        return;
    }

    // Check pointer fields
    for (size_t i = 0; i < struct_info->num_pfields; i++)
    {
        archi_struct_field_info_t pfield = struct_info->pfield[i];

        if (ARCHI_STRING_COMPARE(pfield.name, !=, slot.name))
            continue;

        archi_context_struct_assign_pointer(context_data->structure.ptr,
                &context_data->reference[i], pfield, value, ARCHI_ERROR_PARAMETER);

        return;
    }

    ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

