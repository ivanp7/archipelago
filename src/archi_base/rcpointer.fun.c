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
 * @brief Operations with reference-counted generic pointers.
 */

#include "archi_base/rcpointer.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/ref_count.fun.h"

#include <stdlib.h> // for aligned_alloc(), free()
#include <string.h> // for memcpy()


archi_rcpointer_t
archi_rcpointer(
        archi_pointer_t p,
        archi_reference_count_t ref_count)
{
    return (archi_rcpointer_t){
        .p = p,
        .ref_count = (!ARCHI_POINTER_IS_NULL(p) && !ARCHI_POINTER_TO_TRANS_DATA(p.attr)) ? ref_count : NULL,
    };
}

archi_rcpointer_t
archi_rcpointer_memcopy(
        archi_rcpointer_t source,
        ARCHI_ERROR_PARAM_DECL)
{
    // Check validness of source pointer
    if (!archi_pointer_valid(source.p, ARCHI_ERROR_PARAM))
        return (archi_rcpointer_t){0};

    // Parse source pointer attributes
    size_t length, stride, alignment;
    if (!archi_pointer_attr_unpk__pdata(source.attr, &length, &stride, &alignment,
                ARCHI_ERROR_PARAM))
        return (archi_rcpointer_t){0};

    // Copy the pointer
    archi_rcpointer_t copy = source;

    // Change pointer type
    copy.attr &= ~ARCHI_POINTER_ATTR_MASK__TYPE;
    copy.attr |= ARCHI_POINTER_TYPE__DATA_PERSISTENT_W;

    if (source.ptr != NULL)
    {
        // Allocate the memory copy
        size_t size = length * stride;

        copy.ptr = aligned_alloc(alignment, size);
        if (copy.ptr == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate memory copy (%zu bytes, alignment = %#zx)",
                    size, alignment);
            return (archi_rcpointer_t){0};
        }

        // Allocate the reference counter
        copy.ref_count = archi_reference_count_alloc(free, copy.ptr);
        if (copy.ref_count == NULL)
        {
            free(copy.ptr);

            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate reference counter of memory copy");
            return (archi_rcpointer_t){0};
        }

        // Copy memory contents
        memcpy(copy.ptr, source.ptr, size);
    }

    ARCHI_ERROR_RESET();
    return copy;
}

bool
archi_rcpointer_claim(
        archi_rcpointer_t *entity,
        ARCHI_ERROR_PARAM_DECL)
{
    if (entity == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to claimed entity is NULL");
        return false;
    }

    if (!ARCHI_POINTER_TO_TRANS_DATA(entity->attr))
    {
        archi_reference_count_increment(entity->ref_count);
        ARCHI_ERROR_RESET();
    }
    else
    {
        archi_rcpointer_t entity_copy = archi_rcpointer_memcopy(*entity, ARCHI_ERROR_PARAM);
        if ((entity_copy.ptr == NULL) && (entity->ptr != NULL))
            return false;

        *entity = entity_copy;
    }

    return true;
}

void
archi_rcpointer_abandon(
        archi_rcpointer_t entity)
{
    archi_reference_count_decrement(entity.ref_count);
}

bool
archi_rcpointer_replace(
        archi_rcpointer_t *claimed_entity,
        archi_rcpointer_t abandoned_entity,
        ARCHI_ERROR_PARAM_DECL)
{
    if (!archi_rcpointer_claim(claimed_entity, ARCHI_ERROR_PARAM))
        return false;

    archi_rcpointer_abandon(abandoned_entity);

    return true;
}

