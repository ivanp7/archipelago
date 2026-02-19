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
 * @brief Operations with pointers.
 */

#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/ref_count.fun.h"
#include "archi_base/util/size.def.h"

#include <stdlib.h> // for aligned_alloc(), free()
#include <string.h> // for memcpy()


#define MASK(width) (((archi_pointer_attr_t)1 << (width)) - 1)

static
int
bitwidth_of(
        archi_pointer_attr_t value)
{
    int bitwidth = 0;
    while (value != 0)
    {
        bitwidth++;
        value >>= 1;
    }
    return bitwidth;
}

/*****************************************************************************/
// Pointer attributes interface
/*****************************************************************************/

archi_pointer_attr_t
archi_pointer_attr__pdata(
        size_t length,
        size_t stride,
        size_t alignment,
        ARCHI_ERROR_PARAM_DECL)
{
    // Check validness of inputs
    if (stride == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "stride is zero");
        return -1;
    }
    else if (stride > ARCHI_POINTER_DATA_STRIDE_MAX)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "stride (%zu) is greater than ARCHI_POINTER_DATA_STRIDE_MAX",
                stride);
        return -1;
    }
    else if (ARCHI_SIZE_OVERFLOW(length, stride))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "length * stride (%zu * %zu) overflows size_t", length, stride);
        return -1;
    }
    else if (length * stride > ARCHI_POINTER_DATA_SIZE_MAX)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "length * stride (%zu * %zu) is greater than ARCHI_POINTER_DATA_SIZE_MAX",
                length, stride);
        return -1;
    }
    else if (!ARCHI_ALIGNMENT_VALID(alignment))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "alignment requirement (%#zx) is not a power of two", alignment);
        return -1;
    }
    else if (stride % alignment != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "stride (%zu) is not divisible by alignment requirement (%#zx)",
                stride, alignment);
        return -1;
    }

    // Compute bitwidth of stride
    int stride_width = bitwidth_of(stride - 1);

    // Compute the stride width attribute
    archi_pointer_attr_t attr_stride_width = stride_width;
    attr_stride_width <<= ARCHI_POINTER_ATTR_OFFSET__STRIDE_WIDTH;

    // Compute the stride/alignment attribute
    archi_pointer_attr_t attr_stride_over_alignment = stride / alignment - 1;
    attr_stride_over_alignment <<= ARCHI_POINTER_ATTR_OFFSET__STRIDE_OVER_ALIGNMENT(stride_width);

    // Compute the length attribute
    archi_pointer_attr_t attr_length = length;

    // Compute the whole attributes
    ARCHI_ERROR_RESET();
    return attr_stride_width | attr_stride_over_alignment | attr_length;
}

archi_pointer_attr_t
archi_pointer_attr__cdata(
        archi_pointer_attr_t tag)
{
    if (tag > ARCHI_POINTER_DATA_TAG_MAX)
        return -1;

    return (~tag) & MASK(ARCHI_POINTER_ATTR_OFFSET__TYPE);
}

archi_pointer_attr_t
archi_pointer_attr__func(
        archi_pointer_attr_t tag)
{
    if (tag > ARCHI_POINTER_FUNC_TAG_MAX)
        return 0;

    return ARCHI_POINTER_TYPE__FUNCTION | tag;
}

void
archi_pointer_attr_unpk(
        archi_pointer_attr_t attr,
        unsigned char *out_kind,
        archi_pointer_attr_t *out_tag,
        size_t *out_length,
        size_t *out_stride,
        size_t *out_alignment)
{
    unsigned char kind;

    if (archi_pointer_attr_unpk__pdata(attr, out_length, out_stride, out_alignment, NULL))
        kind = ARCHI_POINTEE__PDATA;
    else if (archi_pointer_attr_unpk__cdata(attr, out_tag, NULL))
        kind = ARCHI_POINTEE__CDATA;
    else if (archi_pointer_attr_unpk__func(attr, out_tag, NULL))
        kind = ARCHI_POINTEE__FUNC;
    else // should not ever happen, listed for robustness
        kind = ARCHI_POINTEE__UNSPEC;

    if (out_kind != NULL)
        *out_kind = kind;
}

bool
archi_pointer_attr_unpk__pdata(
        archi_pointer_attr_t attr,
        size_t *out_length,
        size_t *out_stride,
        size_t *out_alignment,
        ARCHI_ERROR_PARAM_DECL)
{
    // Check pointer type
    if (ARCHI_POINTER_TO_FUNCTION(attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer type is function (expected primitive data)");
        return false;
    }

    // Extract stride attribute bit width
    int stride_width = (attr >> ARCHI_POINTER_ATTR_OFFSET__STRIDE_WIDTH) &
        MASK(ARCHI_POINTER_ATTR_BITWIDTH__STRIDE_WIDTH);

    if (stride_width >= (int)ARCHI_POINTER_ATTR_BITWIDTH__SIZE)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer data type is complex (expected primitive)");
        return false;
    }

    // Write attributes to output parameters
    if (out_length != NULL)
    {
        archi_pointer_attr_t length = attr & MASK(ARCHI_POINTER_ATTR_BITWIDTH__LENGTH(stride_width));

        *out_length = length;
    }

    if ((out_stride != NULL) || (out_alignment != NULL))
    {
        archi_pointer_attr_t stride_over_alignment = MASK(stride_width) &
            (attr >> ARCHI_POINTER_ATTR_OFFSET__STRIDE_OVER_ALIGNMENT(stride_width));

        int alignment_log2 = stride_width - bitwidth_of(stride_over_alignment);

        if (out_alignment != NULL)
            *out_alignment = (size_t)1 << alignment_log2;

        if (out_stride != NULL)
            *out_stride = (stride_over_alignment + 1) << alignment_log2;
    }

    ARCHI_ERROR_RESET();
    return true;
}

bool
archi_pointer_attr_unpk__cdata(
        archi_pointer_attr_t attr,
        archi_pointer_attr_t *out_tag,
        ARCHI_ERROR_PARAM_DECL)
{
    // Check pointer type
    if (ARCHI_POINTER_TO_FUNCTION(attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer type is function (expected complex data)");
        return false;
    }

    // Extract complex data type tag
    archi_pointer_attr_t tag = (~attr) & MASK(ARCHI_POINTER_ATTR_OFFSET__TYPE);

    if (tag > ARCHI_POINTER_DATA_TAG_MAX)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer data type is primitive (expected complex)");
        return false;
    }

    // Write tag to output parameter
    if (out_tag != NULL)
        *out_tag = tag;

    ARCHI_ERROR_RESET();
    return true;
}

bool
archi_pointer_attr_unpk__func(
        archi_pointer_attr_t attr,
        archi_pointer_attr_t *out_tag,
        ARCHI_ERROR_PARAM_DECL)
{
    // Check pointer type
    if (!ARCHI_POINTER_TO_FUNCTION(attr))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer type is data (expected function)");
        return false;
    }

    // Write attributes to output parameters
    if (out_tag != NULL)
    {
        archi_pointer_attr_t tag = attr & MASK(ARCHI_POINTER_ATTR_OFFSET__TYPE);

        *out_tag = tag;
    }

    ARCHI_ERROR_RESET();
    return true;
}

bool
archi_pointer_attr_compatible(
        archi_pointer_attr_t attr,
        archi_pointer_attr_t sample)
{
    archi_pointer_attr_t attr_tag, sample_tag;

    if (!ARCHI_POINTER_TO_FUNCTION(attr) && !ARCHI_POINTER_TO_FUNCTION(sample)) // both are data types
    {
        if ((attr & ~ARCHI_POINTER_TYPE_MASK) == (sample & ~ARCHI_POINTER_TYPE_MASK)) // quick test
            return true;

        bool attr_complex = archi_pointer_attr_unpk__cdata(attr, &attr_tag, NULL);
        bool sample_complex = archi_pointer_attr_unpk__cdata(sample, &sample_tag, NULL);

        if (!attr_complex && !sample_complex) // both are primitive data types
        {
            size_t attr_length, attr_stride, attr_alignment;
            size_t sample_length, sample_stride, sample_alignment;

            archi_pointer_attr_unpk__pdata(attr, &attr_length, &attr_stride, &attr_alignment, NULL);
            archi_pointer_attr_unpk__pdata(sample, &sample_length, &sample_stride, &sample_alignment, NULL);

            return (attr_stride == sample_stride) &&
                (attr_alignment == sample_alignment) &&
                (attr_length >= sample_length);
        }
        else if (attr_complex && sample_complex) // both complex data types
            return (attr_tag == sample_tag) || !attr_tag || !sample_tag;
        else if ((attr_complex && !attr_tag) || (sample_complex && !sample_tag)) // one complex with tag #0, another - primitive
            return true; // tag #0 is compatible with any data type
        else // mismatched types
            return false;
    }
    else if (ARCHI_POINTER_TO_FUNCTION(attr) && ARCHI_POINTER_TO_FUNCTION(sample)) // both function types
    {
        archi_pointer_attr_unpk__func(attr, &attr_tag, NULL);
        archi_pointer_attr_unpk__func(sample, &sample_tag, NULL);

        return (attr_tag == sample_tag) || !attr_tag || !sample_tag;
    }
    else // mismatched types
        return false;
}

/*****************************************************************************/
// Pointer interface
/*****************************************************************************/

bool
archi_pointer_valid(
        archi_pointer_t p,
        ARCHI_ERROR_PARAM_DECL)
{
    if (archi_pointer_attr_unpk__func(p.attr, NULL, NULL))
    {
        ARCHI_ERROR_RESET();
        return true;
    }
    else if (archi_pointer_attr_unpk__cdata(p.attr, NULL, NULL))
    {
        ARCHI_ERROR_RESET();
        return true;
    }
    else
    {
        size_t length, alignment;

        if (!archi_pointer_attr_unpk__pdata(p.attr, &length, NULL, &alignment,
                    ARCHI_ERROR_PARAM))
            return false;

        if (p.ptr != NULL)
        {
            if ((uintptr_t)p.ptr % alignment != 0)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer (%p) is misaligned (required alignment = %#zx)",
                        p.ptr, alignment);
                return false;
            }
        }
        else
        {
            if (length != 0)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer to data of non-zero length (%zu) is NULL",
                        length);
                return false;
            }
        }
    }

    ARCHI_ERROR_RESET();
    return true;
}

archi_rcpointer_t
archi_rcpointer(
        archi_pointer_t p,
        archi_reference_count_t ref_count)
{
    return (archi_rcpointer_t){
        .p = p,
        .ref_count = (!ARCHI_POINTER_IS_NULL(p) && !ARCHI_POINTER_TO_STACK(p.attr)) ? ref_count : NULL,
    };
}

/*****************************************************************************/
// Reference counted pointer interface
/*****************************************************************************/

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
    copy.attr &= ~ARCHI_POINTER_TYPE_MASK;
    copy.attr |= ARCHI_POINTER_TYPE__DATA_WRITABLE;

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

archi_rcpointer_t
archi_rcpointer_own(
        archi_rcpointer_t entity,
        ARCHI_ERROR_PARAM_DECL)
{
    if (!ARCHI_POINTER_TO_STACK(entity.attr))
        archi_reference_count_increment(entity.ref_count);
    else
    {
        archi_rcpointer_t entity_copy = archi_rcpointer_memcopy(entity, ARCHI_ERROR_PARAM);
        if ((entity_copy.ptr == NULL) && (entity.ptr != NULL))
            return (archi_rcpointer_t){.attr = ARCHI_POINTER_TYPE__DATA_ON_STACK};

        entity = entity_copy;
    }

    ARCHI_ERROR_RESET();
    return entity;
}

void
archi_rcpointer_disown(
        archi_rcpointer_t entity)
{
    archi_reference_count_decrement(entity.ref_count);
}

archi_rcpointer_t
archi_rcpointer_own_disown(
        archi_rcpointer_t owned_entity,
        archi_rcpointer_t disowned_entity,
        ARCHI_ERROR_PARAM_DECL)
{
    owned_entity = archi_rcpointer_own(owned_entity, ARCHI_ERROR_PARAM);
    if (!owned_entity.attr)
        return (archi_rcpointer_t){0};

    archi_rcpointer_disown(disowned_entity);

    return owned_entity;
}

