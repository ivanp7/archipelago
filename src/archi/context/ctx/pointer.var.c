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
 * @brief Context interface for pointer wrappers.
 */

#include "archi/context/ctx/pointer.var.h"
#include "archi/context/api/interface.def.h"
#include "archipelago/util/parameters.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/ref_count.fun.h"
#include "archipelago/util/size.def.h"
#include "archipelago/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy(), memmove()

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__pointer)
{
    // Parse parameters
    archi_rcpointer_t entity = {0};
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "pointee", .value_unchecked = true}, // any type
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            entity = parsed[index].value;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    // Own the entity
    *context_data = archi_rcpointer_own(entity, ARCHI_ERROR_PARAMETER);
    if (!context_data->attr) // failed to own
    {
        free(context_data);
        return NULL;
    }

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_FINAL_FUNC(archi_context_final__pointer)
{
    archi_rcpointer_disown(*context);
    free(context);
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__pointer)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(*context);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__pointer)
{
    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        *context = value;
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__pointer = {
    .init_fn = archi_context_init__pointer,
    .final_fn = archi_context_final__pointer,
    .eval_fn = archi_context_eval__pointer,
    .set_fn = archi_context_set__pointer,
};

/*****************************************************************************/

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__dpointer)
{
    // Parse parameters
    archi_rcpointer_t data = {0};
    bool writable = false;
    bool writable_set = false;
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "pointee", .value.attr = archi_pointer_attr__opaque_data(0)}, // any data type
            {.name = "writable", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, char)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            data = parsed[index].value;
        index++;
        writable_set = parsed[index].value_set;
        if (writable_set)
            writable = *(char*)parsed[index].value.ptr;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    // Own the data
    *context_data = archi_rcpointer_own(data, ARCHI_ERROR_PARAMETER);
    if (!context_data->attr) // failed to own
    {
        free(context_data);
        return NULL;
    }

    // Set writability flag
    if (writable_set)
    {
        context_data->attr &= ~ARCHI_POINTER_TYPE_MASK;
        context_data->attr |= writable ? ARCHI_POINTER_TYPE__DATA_WRITABLE :
            ARCHI_POINTER_TYPE__DATA_READONLY;
    }

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__dpointer)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(*context);
    }
    else if (ARCHI_STRING_COMPARE("writable", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        char writable = !ARCHI_POINTER_TO_READONLY_DATA(context->attr);

        archi_rcpointer_t value = {
            .ptr = &writable,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, char),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__dpointer)
{
    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not data");
            return;
        }

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        *context = value;
    }
    else if (ARCHI_STRING_COMPARE("writable", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, char)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a boolean");
            return;
        }

        bool writable = *(char*)value.ptr;

        context->attr &= ~ARCHI_POINTER_TYPE_MASK;
        context->attr |= writable ? ARCHI_POINTER_TYPE__DATA_WRITABLE :
            ARCHI_POINTER_TYPE__DATA_READONLY;
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__dpointer = {
    .init_fn = archi_context_init__dpointer,
    .final_fn = archi_context_final__pointer,
    .eval_fn = archi_context_eval__dpointer,
    .set_fn = archi_context_set__dpointer,
};

/*****************************************************************************/

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__tdpointer)
{
    // Parse parameters
    archi_rcpointer_t data = {0};
    ptrdiff_t offset = 0;
    size_t offset_unit = 1;
    bool writable = false;
    bool writable_set = false;
    size_t length = 0;
    bool length_set = false;
    size_t stride = 1;
    bool stride_set = false;
    size_t alignment = 1;
    bool alignment_set = false;
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "pointee", .value.attr = archi_pointer_attr__opaque_data(0)}, // any data type
            {.name = "offset", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, ptrdiff_t)},
            {.name = "offset_unit", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "writable", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, char)},
            {.name = "length", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "stride", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "alignment", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            data = parsed[index].value;
        index++;
        if (parsed[index].value_set)
            offset = *(ptrdiff_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            offset_unit = *(size_t*)parsed[index].value.ptr;
        index++;
        writable_set = parsed[index].value_set;
        if (writable_set)
            writable = *(char*)parsed[index].value.ptr;
        index++;
        length_set = parsed[index].value_set;
        if (length_set)
            length = *(size_t*)parsed[index].value.ptr;
        index++;
        stride_set = parsed[index].value_set;
        if (stride_set)
            stride = *(size_t*)parsed[index].value.ptr;
        index++;
        alignment_set = parsed[index].value_set;
        if (alignment_set)
            alignment = *(size_t*)parsed[index].value.ptr;
    }

    // Check validity of parameters
    if (offset_unit == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer offset unit is zero");
        return NULL;
    }
    else if (((offset > 0) && ARCHI_SIZE_OVERFLOW((size_t)offset, offset_unit)) ||
            ((offset < 0) && ARCHI_SIZE_OVERFLOW((size_t)-offset, offset_unit)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer offset (%ti) is too large", offset);
        return NULL;
    }

    // Parse pointer attributes
    if ((!length_set || !stride_set || !alignment_set) &&
            !archi_pointer_attr_parse__transp_data(data.attr, (!length_set ? &length : NULL),
                (!stride_set ? &stride : NULL), (!alignment_set ? &alignment : NULL),
                ARCHI_ERROR_PARAMETER))
        return NULL;

    // Check pointer alignment
    if (data.ptr != NULL)
    {
        uintptr_t addr = (uintptr_t)data.ptr;

        if (offset > 0)
            addr += (size_t)offset * offset_unit;
        else if (offset < 0)
            addr -= (size_t)-offset * offset_unit;

        if (addr % alignment != 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "shifted pointer is misaligned");
            return NULL;
        }
    }

    // Construct new pointer attributes
    archi_pointer_attr_t attr = archi_pointer_attr__transp_data(length, stride, alignment,
            ARCHI_ERROR_PARAMETER);
    if (attr == (archi_pointer_attr_t)-1)
        return NULL;

    data.attr = (data.attr & ARCHI_POINTER_TYPE_MASK) | attr;

    // Apply offset
    if (data.ptr != NULL)
    {
        if (offset > 0)
            data.ptr = (char*)data.ptr + (size_t)offset * offset_unit;
        else if (offset < 0)
            data.ptr = (char*)data.ptr - (size_t)-offset * offset_unit;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    // Own the data
    *context_data = archi_rcpointer_own(data, ARCHI_ERROR_PARAMETER);
    if (!context_data->attr) // failed to own
    {
        free(context_data);
        return NULL;
    }

    // Set writability flag
    if (writable_set)
    {
        context_data->attr &= ~ARCHI_POINTER_TYPE_MASK;
        context_data->attr |= writable ? ARCHI_POINTER_TYPE__DATA_WRITABLE :
            ARCHI_POINTER_TYPE__DATA_READONLY;
    }

    // Set pointer attributes
    context_data->attr = (context_data->attr & ARCHI_POINTER_TYPE_MASK) | attr;

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__tdpointer)
{
    if (!call)
    {
        if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            ARCHI_CONTEXT_YIELD(*context);
        }
        else if (ARCHI_STRING_COMPARE("", ==, slot.name))
        {
            if (slot.num_indices != 1)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 1");
                return;
            }

            size_t length, stride, alignment;
            archi_pointer_attr_parse__transp_data(context->attr, &length, &stride, &alignment, NULL);

            ptrdiff_t offset = slot.index[0];
            if ((offset > 0) && ((size_t)offset > length))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "offset (%ti) exceeds data length (%zu)", offset, length);
                return;
            }
            else if ((offset < 0) && (ARCHI_SIZE_OVERFLOW((size_t)-offset, stride) ||
                        ((size_t)-offset * stride > ARCHI_POINTER_DATA_SIZE_MAX) ||
                        (((size_t)-offset + length) * stride > ARCHI_POINTER_DATA_SIZE_MAX)))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "(offset+length)*stride (%ti*%zu+%zu) exceeds ARCHI_POINTER_DATA_SIZE_MAX",
                        offset, stride, length);
                return;
            }

            archi_rcpointer_t value;

            if (context->ptr != NULL)
            {
                value = (archi_rcpointer_t){
                    .ptr = (char*)context->ptr + offset * (ptrdiff_t)stride,
                    .attr = (context->attr & ARCHI_POINTER_TYPE_MASK) |
                        archi_pointer_attr__transp_data(length - offset, stride, alignment, NULL),
                    .ref_count = context->ref_count,
                };
            }
            else
            {
                value = (archi_rcpointer_t){
                    .attr = (context->attr & ARCHI_POINTER_TYPE_MASK) |
                        archi_pointer_attr__transp_data(0, stride, alignment, NULL),
                };
            }

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("writable", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            char writable = !ARCHI_POINTER_TO_READONLY_DATA(context->attr);

            archi_rcpointer_t value = {
                .ptr = &writable,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, char),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("length", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t length;
            archi_pointer_attr_parse__transp_data(context->attr, &length, NULL, NULL, NULL);

            archi_rcpointer_t value = {
                .ptr = &length,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("stride", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t stride;
            archi_pointer_attr_parse__transp_data(context->attr, NULL, &stride, NULL, NULL);

            archi_rcpointer_t value = {
                .ptr = &stride,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("size", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t size;
            {
                size_t length, stride;
                archi_pointer_attr_parse__transp_data(context->attr, &length, &stride, NULL, NULL);
                size = length * stride;
            }

            archi_rcpointer_t value = {
                .ptr = &size,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else if (ARCHI_STRING_COMPARE("alignment", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t alignment;
            archi_pointer_attr_parse__transp_data(context->attr, NULL, NULL, &alignment, NULL);

            archi_rcpointer_t value = {
                .ptr = &alignment,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
            };

            ARCHI_CONTEXT_YIELD(value);
        }
        else
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
    }
    else
    {
        if (ARCHI_STRING_COMPARE("shift_ptr", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t stride, alignment;
            archi_pointer_attr_parse__transp_data(context->attr, NULL, &stride, &alignment, NULL);

            // Parse parameters
            ptrdiff_t offset = 0;
            {
                archi_kvlist_parameter_t parsed[] = {
                    {.name = "offset", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, ptrdiff_t)},
                };

                if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                            ARCHI_ERROR_PARAMETER))
                    return;

                size_t index = 0;
                if (parsed[index].value_set)
                    offset = *(ptrdiff_t*)parsed[index].value.ptr;
            }

            // Check validity of parameters
            if (((offset > 0) && ARCHI_SIZE_OVERFLOW((size_t)offset, stride)) ||
                    ((offset < 0) && ARCHI_SIZE_OVERFLOW((size_t)-offset, stride)))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer offset (%ti) is too large", offset);
                return;
            }

            // Apply offset
            if (context->ptr != NULL)
            {
                if (offset > 0)
                    context->ptr = (char*)context->ptr + (size_t)offset * stride;
                else if (offset < 0)
                    context->ptr = (char*)context->ptr - (size_t)-offset * stride;
            }
        }
        else if (ARCHI_STRING_COMPARE("set_attr", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t length, stride, alignment;
            archi_pointer_attr_parse__transp_data(context->attr, &length, &stride, &alignment, NULL);

            // Parse parameters
            {
                archi_kvlist_parameter_t parsed[] = {
                    {.name = "length", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
                    {.name = "stride", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
                    {.name = "alignment", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
                };

                if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                            ARCHI_ERROR_PARAMETER))
                    return;

                size_t index = 0;
                if (parsed[index].value_set)
                    length = *(size_t*)parsed[index].value.ptr;
                index++;
                if (parsed[index].value_set)
                    stride = *(size_t*)parsed[index].value.ptr;
                index++;
                if (parsed[index].value_set)
                    alignment = *(size_t*)parsed[index].value.ptr;
            }

            // Check validity of parameters
            archi_pointer_attr_t attr = archi_pointer_attr__transp_data(length, stride, alignment,
                    ARCHI_ERROR_PARAMETER);
            if (attr == (archi_pointer_attr_t)-1)
                return;

            if ((context->ptr != NULL) && ((uintptr_t)context->ptr % alignment != 0))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned alignment (%zu) is greater then pointer alignment",
                        alignment);
                return;
            }

            context->attr = (context->attr & ARCHI_POINTER_TYPE_MASK) | attr;
        }
        else if (ARCHI_STRING_COMPARE("copy", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }
            else if (ARCHI_POINTER_TO_READONLY_DATA(context->attr))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination memory is read-only");
                return;
            }

            size_t dest_length, stride, alignment;
            archi_pointer_attr_parse__transp_data(context->attr, &dest_length, &stride, &alignment, NULL);

            // Parse parameters
            archi_rcpointer_t source = {0};
            bool source_set = false;
            size_t src_offset = 0;
            size_t dest_offset = 0;
            size_t copy_length = 0;
            bool copy_length_set = false;
            {
                archi_kvlist_parameter_t parsed[] = {
                    {.name = "from", .value.attr = archi_pointer_attr__transp_data(0, stride, alignment, NULL)},
                    {.name = "from_offset", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
                    {.name = "offset", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
                    {.name = "length", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
                };

                if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                            ARCHI_ERROR_PARAMETER))
                    return;

                size_t index = 0;
                source_set = parsed[index].value_set;
                if (source_set)
                    source = parsed[index].value;
                index++;
                if (parsed[index].value_set)
                    src_offset = *(size_t*)parsed[index].value.ptr;
                index++;
                if (parsed[index].value_set)
                    dest_offset = *(size_t*)parsed[index].value.ptr;
                index++;
                copy_length_set = parsed[index].value_set;
                if (copy_length_set)
                    copy_length = *(size_t*)parsed[index].value.ptr;
            }

            // Check validity of parameters
            if (!source_set)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source pointer is not set");
                return;
            }

            size_t src_length;
            {
                archi_error_t error;
                if (!archi_pointer_attr_parse__transp_data(source.attr, &src_length, NULL, NULL, &error))
                {
                    ARCHI_ERROR_SET(error.code, "source pointer does not refer to transparent data: %s", error.message);
                    return;
                }
            }

            if (src_offset > src_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source offset (%zu) exceeds source length (%zu)",
                        src_offset, src_length);
                return;
            }

            if (!copy_length_set)
                copy_length = src_length - src_offset;

            if (copy_length != 0)
            {
                if (source.ptr == NULL)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source pointer is NULL for non-zero copy length");
                    return;
                }
                else if (context->ptr == NULL)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination pointer is NULL for non-zero copy length");
                    return;
                }
            }

            if (src_offset + copy_length > src_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "copied area (offset = %zu, length = %zu) is out of source bounds (length = %zu)",
                        src_offset, copy_length, src_length);
                return;
            }
            else if (dest_offset + copy_length > dest_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "copied area (offset = %zu, length = %zu) is out of destination bounds (length = %zu)",
                        dest_offset, copy_length, dest_length);
                return;
            }

            // Copy the memory
            if (copy_length != 0)
                memmove((char*)context->ptr + dest_offset * stride,
                        (const char*)source.cptr + src_offset * stride,
                        copy_length * stride);
        }
        else if (ARCHI_STRING_COMPARE("fill", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }
            else if (ARCHI_POINTER_TO_READONLY_DATA(context->attr))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination memory is read-only");
                return;
            }

            size_t length, stride, alignment;
            archi_pointer_attr_parse__transp_data(context->attr, &length, &stride, &alignment, NULL);

            // Parse parameters
            archi_rcpointer_t pattern = {0};
            bool pattern_set = false;
            size_t offset = 0;
            size_t fill_length = 0;
            bool fill_length_set = false;
            {
                archi_kvlist_parameter_t parsed[] = {
                    {.name = "pattern", .value.attr = archi_pointer_attr__transp_data(1, stride, alignment, NULL)},
                    {.name = "offset", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
                    {.name = "length", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
                };

                if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                            ARCHI_ERROR_PARAMETER))
                    return;

                size_t index = 0;
                pattern_set = parsed[index].value_set;
                if (pattern_set)
                    pattern = parsed[index].value;
                index++;
                if (parsed[index].value_set)
                    offset = *(size_t*)parsed[index].value.ptr;
                index++;
                fill_length_set = parsed[index].value_set;
                if (fill_length_set)
                    fill_length = *(size_t*)parsed[index].value.ptr;
            }

            // Check validity of parameters
            if (!pattern_set)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pattern pointer is not set");
                return;
            }

            if (offset > length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination offset (%zu) exceeds destination length (%zu)",
                        offset, length);
                return;
            }

            if (!fill_length_set)
                fill_length = length;

            size_t pattern_length;
            archi_pointer_attr_parse__transp_data(pattern.attr, &pattern_length, NULL, NULL, NULL);

            if (fill_length % pattern_length != 0)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "filled destination length (%zu) is not divisible by pattern length (%zu)",
                        fill_length, pattern_length);
                return;
            }

            // Fill the memory
            size_t num_patterns = fill_length / pattern_length;
            for (size_t i = 0; i < num_patterns; i++)
                memcpy((char*)context->ptr + (offset + pattern_length * i) * stride,
                        pattern.cptr,
                        pattern_length * stride);
        }
        else
        {
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
            return;
        }

        ARCHI_ERROR_RESET();
    }
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__tdpointer)
{
    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_parse__transp_data(value.attr, NULL, NULL, NULL, ARCHI_ERROR_PARAMETER))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not transparent data");
            return;
        }

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        *context = value;
    }
    else if (ARCHI_STRING_COMPARE("writable", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, char)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a boolean");
            return;
        }

        bool writable = *(char*)value.ptr;

        context->attr &= ~ARCHI_POINTER_TYPE_MASK;
        context->attr |= writable ? ARCHI_POINTER_TYPE__DATA_WRITABLE :
            ARCHI_POINTER_TYPE__DATA_READONLY;
    }
    else if (ARCHI_STRING_COMPARE("length", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a size_t");
            return;
        }

        size_t length = *(size_t*)value.ptr;

        size_t stride, alignment;
        archi_pointer_attr_parse__transp_data(context->attr, NULL, &stride, &alignment, NULL);

        archi_pointer_attr_t attr = archi_pointer_attr__transp_data(length, stride, alignment,
                ARCHI_ERROR_PARAMETER);
        if (attr == (archi_pointer_attr_t)-1)
            return;

        context->attr = (context->attr & ARCHI_POINTER_TYPE_MASK) | attr;
    }
    else if (ARCHI_STRING_COMPARE("stride", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a size_t");
            return;
        }

        size_t stride = *(size_t*)value.ptr;
        if (stride == 0)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned stride is 0");
            return;
        }

        size_t length, alignment;
        {
            size_t old_stride;
            archi_pointer_attr_parse__transp_data(context->attr, &length, &old_stride, &alignment, NULL);

            size_t size = length * old_stride;
            if (size % stride != 0)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data size is not divisible by new stride");
                return;
            }

            length = size / stride;
        }

        archi_pointer_attr_t attr = archi_pointer_attr__transp_data(length, stride, alignment,
                ARCHI_ERROR_PARAMETER);
        if (attr == (archi_pointer_attr_t)-1)
            return;

        context->attr = (context->attr & ARCHI_POINTER_TYPE_MASK) | attr;
    }
    else if (ARCHI_STRING_COMPARE("alignment", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a size_t");
            return;
        }

        size_t alignment = *(size_t*)value.ptr;

        size_t length, stride;
        archi_pointer_attr_parse__transp_data(context->attr, &length, &stride, NULL, NULL);

        archi_pointer_attr_t attr = archi_pointer_attr__transp_data(length, stride, alignment,
                ARCHI_ERROR_PARAMETER);
        if (attr == (archi_pointer_attr_t)-1)
            return;

        if ((context->ptr != NULL) && ((uintptr_t)context->ptr % alignment != 0))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned alignment (%zu) is greater then pointer alignment",
                    alignment);
            return;
        }

        context->attr = (context->attr & ARCHI_POINTER_TYPE_MASK) | attr;
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__tdpointer = {
    .init_fn = archi_context_init__tdpointer,
    .final_fn = archi_context_final__pointer,
    .eval_fn = archi_context_eval__tdpointer,
    .set_fn = archi_context_set__tdpointer,
};

/*****************************************************************************/

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__odpointer)
{
    // Parse parameters
    archi_rcpointer_t data = {0};
    ptrdiff_t offset = 0;
    size_t offset_unit = 1;
    bool writable = false;
    bool writable_set = false;
    archi_pointer_attr_t tag = 0;
    bool tag_set = false;
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "pointee", .value.attr = archi_pointer_attr__opaque_data(0)}, // any data type
            {.name = "offset", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, ptrdiff_t)},
            {.name = "offset_unit", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t)},
            {.name = "writable", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, char)},
            {.name = "tag", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_pointer_attr_t)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            data = parsed[index].value;
        index++;
        if (parsed[index].value_set)
            offset = *(ptrdiff_t*)parsed[index].value.ptr;
        index++;
        if (parsed[index].value_set)
            offset_unit = *(size_t*)parsed[index].value.ptr;
        index++;
        writable_set = parsed[index].value_set;
        if (writable_set)
            writable = *(char*)parsed[index].value.ptr;
        index++;
        tag_set = parsed[index].value_set;
        if (tag_set)
            tag = *(archi_pointer_attr_t*)parsed[index].value.ptr;
    }

    // Check validity of parameters
    if (offset_unit == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer offset unit is zero");
        return NULL;
    }
    else if (((offset > 0) && ARCHI_SIZE_OVERFLOW((size_t)offset, offset_unit)) ||
            ((offset < 0) && ARCHI_SIZE_OVERFLOW((size_t)-offset, offset_unit)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer offset (%ti) is too large", offset);
        return NULL;
    }
    else if (tag > ARCHI_POINTER_DATA_TAG_MAX)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "opaque data type tag (%llu) exceeds ARCHI_POINTER_DATA_TAG_MAX",
                (unsigned long long)tag);
        return NULL;
    }

    // Apply offset
    if (data.ptr != NULL)
    {
        if (offset > 0)
            data.ptr = (char*)data.ptr + (size_t)offset * offset_unit;
        else if (offset < 0)
            data.ptr = (char*)data.ptr - (size_t)-offset * offset_unit;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    // Own the data
    *context_data = archi_rcpointer_own(data, ARCHI_ERROR_PARAMETER);
    if (!context_data->attr) // failed to own
    {
        free(context_data);
        return NULL;
    }

    // Set writability flag
    if (writable_set)
    {
        context_data->attr &= ~ARCHI_POINTER_TYPE_MASK;
        context_data->attr |= writable ? ARCHI_POINTER_TYPE__DATA_WRITABLE :
            ARCHI_POINTER_TYPE__DATA_READONLY;
    }

    // Set data type tag
    if (tag_set || !archi_pointer_attr_parse__opaque_data(context_data->attr, NULL, NULL))
        context_data->attr = (context_data->attr & ARCHI_POINTER_TYPE_MASK) | archi_pointer_attr__opaque_data(tag);

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__odpointer)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(*context);
    }
    else if (ARCHI_STRING_COMPARE("writable", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        char writable = !ARCHI_POINTER_TO_READONLY_DATA(context->attr);

        archi_rcpointer_t value = {
            .ptr = &writable,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, char),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else if (ARCHI_STRING_COMPARE("tag", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_pointer_attr_t tag;
        if (!archi_pointer_attr_parse__opaque_data(context->attr, &tag, ARCHI_ERROR_PARAMETER))
            return;

        archi_rcpointer_t value = {
            .ptr = &tag,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_pointer_attr_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__odpointer)
{
    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not data");
            return;
        }

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        if (!archi_pointer_attr_parse__opaque_data(value.attr, NULL, NULL))
            value.attr = (value.attr & ARCHI_POINTER_TYPE_MASK) | archi_pointer_attr__opaque_data(0);

        *context = value;
    }
    else if (ARCHI_STRING_COMPARE("writable", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, char)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a boolean");
            return;
        }

        bool writable = *(char*)value.ptr;

        context->attr &= ~ARCHI_POINTER_TYPE_MASK;
        context->attr |= writable ? ARCHI_POINTER_TYPE__DATA_WRITABLE :
            ARCHI_POINTER_TYPE__DATA_READONLY;
    }
    else if (ARCHI_STRING_COMPARE("tag", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_pointer_attr_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not pointer attributes");
            return;
        }

        archi_pointer_attr_t tag = *(archi_pointer_attr_t*)value.ptr;
        if (tag > ARCHI_POINTER_DATA_TAG_MAX)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "opaque data type tag (%llu) exceeds ARCHI_POINTER_DATA_TAG_MAX",
                    (unsigned long long)tag);
            return;
        }

        context->attr = (context->attr & ARCHI_POINTER_TYPE_MASK) | archi_pointer_attr__opaque_data(tag);
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__odpointer = {
    .init_fn = archi_context_init__odpointer,
    .final_fn = archi_context_final__pointer,
    .eval_fn = archi_context_eval__odpointer,
    .set_fn = archi_context_set__odpointer,
};

/*****************************************************************************/

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__fpointer)
{
    // Parse parameters
    archi_rcpointer_t function = {.attr = archi_pointer_attr__function(0)};
    archi_pointer_attr_t tag = 0;
    bool tag_set = false;
    {
        archi_kvlist_parameter_t parsed[] = {
            {.name = "pointee", .value.attr = archi_pointer_attr__function(0)}, // any function type
            {.name = "tag", .value.attr = ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_pointer_attr_t)},
        };

        if (!archi_kvlist_parameters_parse(params, parsed, ARCHI_LENGTH_ARRAY(parsed), false, NULL,
                    ARCHI_ERROR_PARAMETER))
            return NULL;

        size_t index = 0;
        if (parsed[index].value_set)
            function = parsed[index].value;
        index++;
        tag_set = parsed[index].value_set;
        if (tag_set)
            tag = *(archi_pointer_attr_t*)parsed[index].value.ptr;
    }

    // Check validity of parameters
    if (tag > ARCHI_POINTER_FUNCTION_TAG_MAX)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "function type tag (%llu) exceeds ARCHI_POINTER_FUNCTION_TAG_MAX",
                (unsigned long long)tag);
        return NULL;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    // Own the data
    *context_data = archi_rcpointer_own(function, ARCHI_ERROR_PARAMETER);
    if (!context_data->attr) // failed to own
    {
        free(context_data);
        return NULL;
    }

    // Set function type tag
    if (tag_set)
        context_data->attr = archi_pointer_attr__function(tag);

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__fpointer)
{
    (void) params;

    if (call)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "no calls are supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        ARCHI_CONTEXT_YIELD(*context);
    }
    else if (ARCHI_STRING_COMPARE("tag", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        archi_pointer_attr_t tag;
        if (!archi_pointer_attr_parse__function(context->attr, &tag, ARCHI_ERROR_PARAMETER))
            return;

        archi_rcpointer_t value = {
            .ptr = &tag,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_pointer_attr_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__fpointer)
{
    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!ARCHI_POINTER_TO_FUNCTION(value.attr))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a function");
            return;
        }

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAMETER);
        if (!value.attr) // failed to own
            return;

        *context = value;
    }
    else if (ARCHI_STRING_COMPARE("tag", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_compatible(value.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_pointer_attr_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not pointer attributes");
            return;
        }

        archi_pointer_attr_t tag = *(archi_pointer_attr_t*)value.ptr;
        if (tag > ARCHI_POINTER_FUNCTION_TAG_MAX)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "function type tag (%llu) exceeds ARCHI_POINTER_FUNCTION_TAG_MAX",
                    (unsigned long long)tag);
            return;
        }

        context->attr = archi_pointer_attr__function(tag);
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__fpointer = {
    .init_fn = archi_context_init__fpointer,
    .final_fn = archi_context_final__pointer,
    .eval_fn = archi_context_eval__fpointer,
    .set_fn = archi_context_set__fpointer,
};

