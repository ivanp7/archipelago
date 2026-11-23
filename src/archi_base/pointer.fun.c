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
 * @brief Operations with generic pointers.
 */

#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/util/size.def.h"

#include <stdint.h> // for uintmax_t


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
// Pointer attributes operations
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
    else if (stride > ARCHI_POINTER_ATTR_MAX__DATA_STRIDE)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "stride (%zu) is greater than ARCHI_POINTER_ATTR_MAX__DATA_STRIDE",
                stride);
        return -1;
    }
    else if (ARCHI_SIZE_OVERFLOW(length, stride))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "length * stride (%zu * %zu) overflows size_t", length, stride);
        return -1;
    }
    else if (length * stride > ARCHI_POINTER_ATTR_MAX__DATA_SIZE)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "length * stride (%zu * %zu) is greater than ARCHI_POINTER_ATTR_MAX__DATA_SIZE",
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

    // Compute width of the stride/alignment field
    int stride_align_width = bitwidth_of(stride - 1);

    // Compute the primitive data shape split field
    int shape_split = MASK(ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE_SPLIT) - stride_align_width;

    archi_pointer_attr_t field_shape_split = (archi_pointer_attr_t)shape_split
        << ARCHI_POINTER_ATTR_OFFSET__PDATA_SHAPE_SPLIT;

    // Compute the stride/alignment field
    archi_pointer_attr_t field_stride_align = (stride / alignment - 1)
        << ARCHI_POINTER_ATTR_OFFSET__PDATA_STRIDE_ALIGN(shape_split);

    // Compute the length attribute
    archi_pointer_attr_t field_length = length;

    // Compute the whole attributes
    ARCHI_ERROR_RESET();
    return field_shape_split | field_stride_align | field_length;
}

archi_pointer_attr_t
archi_pointer_attr__cdata(
        archi_pointer_attr_t tag)
{
    if (tag > ARCHI_POINTER_ATTR_MAX__DATA_TAG)
        return -1;

    return tag;
}

archi_pointer_attr_t
archi_pointer_attr__func(
        archi_pointer_attr_t tag)
{
    if (tag > ARCHI_POINTER_ATTR_MAX__FUNC_TAG)
        return 0;

    return ARCHI_POINTEE_MEM__FUNCTION | tag;
}

void
archi_pointer_attr_unpk(
        archi_pointer_attr_t attr,
        unsigned char *out_type,
        archi_pointer_attr_t *out_tag,
        size_t *out_length,
        size_t *out_stride,
        size_t *out_alignment)
{
    unsigned char type = ARCHI_POINTEE_TYPE__UNSPEC;

    if (archi_pointer_attr_unpk__pdata(attr, out_length, out_stride, out_alignment, NULL))
        type = ARCHI_POINTEE_TYPE__PDATA;
    else if (archi_pointer_attr_unpk__cdata(attr, out_tag, NULL))
        type = ARCHI_POINTEE_TYPE__CDATA;
    else if (archi_pointer_attr_unpk__func(attr, out_tag, NULL))
        type = ARCHI_POINTEE_TYPE__FUNC;

    if (out_type != NULL)
        *out_type = type;
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

    // Extract primitive data shape split
    int shape_split = (attr >> ARCHI_POINTER_ATTR_OFFSET__PDATA_SHAPE_SPLIT) &
        MASK(ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE_SPLIT);

    int stride_align_width = ARCHI_POINTER_ATTR_WIDTH__PDATA_STRIDE_ALIGN(shape_split);
    if (stride_align_width >= ARCHI_POINTER_ATTR_WIDTH__PDATA_SHAPE)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer data type is complex (expected primitive)");
        return false;
    }

    // Write attributes to output parameters
    if (out_length != NULL)
    {
        archi_pointer_attr_t length = attr &
            MASK(ARCHI_POINTER_ATTR_WIDTH__PDATA_LENGTH(shape_split));

        *out_length = length;
    }

    if ((out_stride != NULL) || (out_alignment != NULL))
    {
        archi_pointer_attr_t stride_align = MASK(stride_align_width) &
            (attr >> ARCHI_POINTER_ATTR_OFFSET__PDATA_STRIDE_ALIGN(shape_split));

        int alignment_log2 = stride_align_width - bitwidth_of(stride_align);

        if (out_stride != NULL)
            *out_stride = (size_t)(stride_align + 1) << alignment_log2;

        if (out_alignment != NULL)
            *out_alignment = (size_t)1 << alignment_log2;
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

    // Check data type
    if (((attr >> ARCHI_POINTER_ATTR_OFFSET__DATA_TYPE) &
                MASK(ARCHI_POINTER_ATTR_WIDTH__DATA_TYPE)) != 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer data type is primitive (expected complex)");
        return false;
    }

    // Write tag to output parameter
    if (out_tag != NULL)
    {
        archi_pointer_attr_t tag = attr & MASK(ARCHI_POINTER_ATTR_WIDTH__DATA_TAG);

        *out_tag = tag;
    }

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
        archi_pointer_attr_t tag = attr & MASK(ARCHI_POINTER_ATTR_WIDTH__FUNC_TAG);

        *out_tag = tag;
    }

    ARCHI_ERROR_RESET();
    return true;
}

bool
archi_pointer_attr_compatible(
        archi_pointer_attr_t attr,
        archi_pointer_attr_t sample,
        ARCHI_ERROR_PARAM_DECL)
{
    archi_pointer_attr_t attr_tag, sample_tag;
    bool attr_is_function = ARCHI_POINTER_TO_FUNCTION(attr),
         sample_is_function = ARCHI_POINTER_TO_FUNCTION(sample);

    if (!attr_is_function && !sample_is_function)
    {
         // quick check if data attributes are the same (ignoring memory kind)
        if ((attr & ~ARCHI_POINTER_ATTR_MASK__MEM_KIND) == (sample & ~ARCHI_POINTER_ATTR_MASK__MEM_KIND))
            goto compatible;

        bool attr_is_complex = archi_pointer_attr_unpk__cdata(attr, &attr_tag, NULL);
        bool sample_is_complex = archi_pointer_attr_unpk__cdata(sample, &sample_tag, NULL);

        if ((attr_is_complex && !attr_tag) || (sample_is_complex && !sample_tag)) // wildcard
            goto compatible; // wildcard is compatible with any data type, primitive or complex

        if (attr_is_complex && sample_is_complex)
        {
            if (!attr_tag || !sample_tag) // wildcard
                goto compatible;

            int mask_width = bitwidth_of(sample_tag) + ARCHI_POINTER_ATTR_WIDTH__DATA_TAG_SEP;
            if (mask_width > (int)ARCHI_POINTER_ATTR_WIDTH__DATA_TAG)
                mask_width = ARCHI_POINTER_ATTR_WIDTH__DATA_TAG;

            if (((attr_tag ^ sample_tag) & MASK(mask_width)) != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EATTRIBUTES, "data tag is incorrect (have = %#jx, need = %#jx)",
                        (uintmax_t)attr_tag, (uintmax_t)sample_tag);
                return false;
            }
        }
        else if (!attr_is_complex && !sample_is_complex)
        {
            size_t attr_length, attr_stride, attr_alignment;
            size_t sample_length, sample_stride, sample_alignment;

            archi_pointer_attr_unpk__pdata(attr, &attr_length, &attr_stride, &attr_alignment, NULL);
            archi_pointer_attr_unpk__pdata(sample, &sample_length, &sample_stride, &sample_alignment, NULL);

            if (attr_stride != sample_stride)
            {
                ARCHI_ERROR_SET(ARCHI__EATTRIBUTES, "data stride is incorrect (have = %zu, need = %zu)",
                        attr_stride, sample_stride);
                return false;
            }
            else if (attr_alignment != sample_alignment)
            {
                ARCHI_ERROR_SET(ARCHI__EATTRIBUTES, "data alignment is incorrect (have = %zu, need = %zu)",
                        attr_alignment, sample_alignment);
                return false;
            }
            else if (attr_length < sample_length)
            {
                ARCHI_ERROR_SET(ARCHI__EATTRIBUTES, "data length is too short (have = %zu, need = %zu)",
                        attr_length, sample_length);
                return false;
            }
        }
        else // one type is complex, another is primitive
        {
            if (!attr_is_complex)
                ARCHI_ERROR_SET(ARCHI__EATTRIBUTES, "data type is not complex");
            else
                ARCHI_ERROR_SET(ARCHI__EATTRIBUTES, "data type is not primitive");

            return false;
        }
    }
    else if (attr_is_function && sample_is_function)
    {
        archi_pointer_attr_unpk__func(attr, &attr_tag, NULL);
        archi_pointer_attr_unpk__func(sample, &sample_tag, NULL);

        if (!attr_tag || !sample_tag) // wildcard
            goto compatible;

        if (attr_tag != sample_tag)
        {
            ARCHI_ERROR_SET(ARCHI__EATTRIBUTES, "function tag is incorrect (have = %#jx, need = %#jx)",
                    (uintmax_t)attr_tag, (uintmax_t)sample_tag);
            return false;
        }
    }
    else // one type is data, another is function
    {
        if (!attr_is_function)
            ARCHI_ERROR_SET(ARCHI__EATTRIBUTES, "value is not a function");
        else
            ARCHI_ERROR_SET(ARCHI__EATTRIBUTES, "value is not data");

        return false;
    }

compatible:
    ARCHI_ERROR_RESET();
    return true;
}

/*****************************************************************************/
// Pointer operations
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

