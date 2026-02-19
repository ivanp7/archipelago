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
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_base/tag.def.h"
#include "archi_base/util/plist.fun.h"
#include "archi_base/util/check.fun.h"
#include "archi_base/util/string.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy(), memmove()


static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__pointer)
{
    // Parse parameters
    archi_rcpointer_t entity = {0};
    {
        archi_plist_param_t parsed[] = {
            {.name = "pointee",
                .assign = {archi_plist_assign__rcpointer, &entity, sizeof(entity)}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Construct the context
    archi_rcpointer_t *context_data = malloc(sizeof(*context_data));
    if (context_data == NULL)
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate context data");
        return NULL;
    }

    // Own the entity
    *context_data = archi_rcpointer_own(entity, ARCHI_ERROR_PARAM);
    if (!context_data->attr)
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
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAM);
        if (!value.attr)
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
        archi_plist_param_t parsed[] = {
            {.name = "pointee",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                .assign = {archi_plist_assign__rcpointer, &data, sizeof(data)}},
            {.name = "writable",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &writable, sizeof(writable), &writable_set}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
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
    *context_data = archi_rcpointer_own(data, ARCHI_ERROR_PARAM);
    if (!context_data->attr)
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
                ARCHI_POINTER_ATTR__PDATA(1, char),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__dpointer)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

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

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAM);
        if (!value.attr)
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
                    ARCHI_POINTER_ATTR__PDATA(1, char)))
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
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__pdpointer)
{
    // Parse parameters
    archi_rcpointer_t data = {0};
    long long offset = 0;
    size_t offset_unit = 1;
    bool writable = false;
    size_t length = 0, stride = 1, alignment = 1;
    bool writable_set = false, length_set = false, stride_set = false, alignment_set = false;
    {
        archi_plist_param_t parsed[] = {
            {.name = "pointee",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                .assign = {archi_plist_assign__rcpointer, &data, sizeof(data)}},
            {.name = "offset",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, long long)}},
                .assign = {archi_plist_assign__value, &offset, sizeof(offset)}},
            {.name = "offset_unit",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &offset_unit, sizeof(offset_unit)}},
            {.name = "writable",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &writable, sizeof(writable), &writable_set}},
            {.name = "length",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &length, sizeof(length), &length_set}},
            {.name = "stride",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &stride, sizeof(stride), &stride_set}},
            {.name = "alignment",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &alignment, sizeof(alignment), &alignment_set}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Check validness of parameters
    if (offset_unit == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer offset unit is zero");
        return NULL;
    }

    // Parse pointer attributes
    if ((!length_set || !stride_set || !alignment_set) &&
            !archi_pointer_attr_unpk__pdata(data.attr, (!length_set ? &length : NULL),
                (!stride_set ? &stride : NULL), (!alignment_set ? &alignment : NULL),
                ARCHI_ERROR_PARAM))
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
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer is misaligned");
            return NULL;
        }
    }

    // Construct new pointer attributes
    archi_pointer_attr_t attr = archi_pointer_attr__pdata(length, stride, alignment,
            ARCHI_ERROR_PARAM);
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
    *context_data = archi_rcpointer_own(data, ARCHI_ERROR_PARAM);
    if (!context_data->attr)
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
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__pdpointer)
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
            archi_pointer_attr_unpk__pdata(context->attr, &length, &stride, &alignment, NULL);

            archi_context_slot_index_t offset = slot.index[0];
            if ((offset > 0) && ((size_t)offset > length))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "offset (%lli) exceeds data length (%zu)", offset, length);
                return;
            }
            else if ((offset < 0) && (length - offset >= (ARCHI_POINTER_DATA_SIZE_MAX + 1) / stride))
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "(offset+length)*stride (%lli*%zu+%zu) exceeds ARCHI_POINTER_DATA_SIZE_MAX",
                        offset, stride, length);
                return;
            }

            archi_rcpointer_t value;

            if (context->ptr != NULL)
            {
                value = (archi_rcpointer_t){
                    .ptr = (char*)context->ptr + (ptrdiff_t)offset * (ptrdiff_t)stride,
                    .attr = (context->attr & ARCHI_POINTER_TYPE_MASK) |
                        archi_pointer_attr__pdata(length - offset, stride, alignment, NULL),
                    .ref_count = context->ref_count,
                };
            }
            else
            {
                value = (archi_rcpointer_t){
                    .attr = (context->attr & ARCHI_POINTER_TYPE_MASK) |
                        archi_pointer_attr__pdata(0, stride, alignment, NULL),
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
                    ARCHI_POINTER_ATTR__PDATA(1, char),
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
            archi_pointer_attr_unpk__pdata(context->attr, &length, NULL, NULL, NULL);

            archi_rcpointer_t value = {
                .ptr = &length,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, size_t),
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
            archi_pointer_attr_unpk__pdata(context->attr, NULL, &stride, NULL, NULL);

            archi_rcpointer_t value = {
                .ptr = &stride,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, size_t),
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
                archi_pointer_attr_unpk__pdata(context->attr, &length, &stride, NULL, NULL);
                size = length * stride;
            }

            archi_rcpointer_t value = {
                .ptr = &size,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, size_t),
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
            archi_pointer_attr_unpk__pdata(context->attr, NULL, NULL, &alignment, NULL);

            archi_rcpointer_t value = {
                .ptr = &alignment,
                .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                    ARCHI_POINTER_ATTR__PDATA(1, size_t),
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
            archi_pointer_attr_unpk__pdata(context->attr, NULL, &stride, &alignment, NULL);

            // Parse parameters
            long long offset = 0;
            {
                archi_plist_param_t parsed[] = {
                    {.name = "offset",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, long long)}},
                        .assign = {archi_plist_assign__value, &offset, sizeof(offset)}},
                    {0},
                };

                if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
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

            ARCHI_ERROR_RESET();
        }
        else if (ARCHI_STRING_COMPARE("set_attr", ==, slot.name))
        {
            if (slot.num_indices != 0)
            {
                ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
                return;
            }

            size_t length, stride, alignment;
            archi_pointer_attr_unpk__pdata(context->attr, &length, &stride, &alignment, NULL);

            // Parse parameters
            {
                archi_plist_param_t parsed[] = {
                    {.name = "length",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &length, sizeof(length)}},
                    {.name = "stride",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &stride, sizeof(stride)}},
                    {.name = "alignment",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &alignment, sizeof(alignment)}},
                    {0},
                };

                if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
                    return;
            }

            // Check validness of parameters
            archi_pointer_attr_t attr = archi_pointer_attr__pdata(length, stride, alignment,
                    ARCHI_ERROR_PARAM);
            if (attr == (archi_pointer_attr_t)-1)
                return;

            if ((context->ptr != NULL) && ((uintptr_t)context->ptr % alignment != 0))
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer is misaligned");
                return;
            }

            context->attr = (context->attr & ARCHI_POINTER_TYPE_MASK) | attr;

            ARCHI_ERROR_RESET();
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

            size_t dest_length, dest_stride;
            archi_pointer_attr_unpk__pdata(context->attr, &dest_length, &dest_stride, NULL, NULL);

            // Parse parameters
            archi_rcpointer_t source = {0};
            size_t src_offset = 0, dest_offset = 0, copy_length = 0;
            bool source_set = false, copy_length_set = false;
            {
                archi_plist_param_t parsed[] = {
                    {.name = "src",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                        .assign = {archi_plist_assign__rcpointer, &source, sizeof(source), &source_set}},
                    {.name = "src_offset",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &src_offset, sizeof(src_offset)}},
                    {.name = "offset",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &dest_offset, sizeof(dest_offset)}},
                    {.name = "length",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &copy_length, sizeof(copy_length), &copy_length_set}},
                    {0},
                };

                if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
                    return;
            }

            // Check validness of parameters
            if (!source_set)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source pointer is not set");
                return;
            }

            size_t src_length, src_stride;
            {
                ARCHI_ERROR_VAR(error);

                if (!archi_pointer_attr_unpk__pdata(source.attr, &src_length, &src_stride, NULL, &error))
                {
                    ARCHI_ERROR_SET(error.code, "source is not primitive data: %s", error.message);
                    return;
                }
            }

            if (dest_offset > dest_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination offset (%zu) exceeds destination length (%zu)",
                        dest_offset, dest_length);
                return;
            }
            else if (src_offset > src_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source offset (%zu) exceeds source length (%zu)",
                        src_offset, src_length);
                return;
            }

            if (!copy_length_set)
                copy_length = dest_length - dest_offset;

            if (copy_length != 0)
            {
                if (context->ptr == NULL)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "destination pointer is NULL");
                    return;
                }
                else if (source.ptr == NULL)
                {
                    ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source pointer is NULL");
                    return;
                }
            }

            size_t copy_size = copy_length * dest_stride;

            if (copy_size % src_stride != 0)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "copied area size (%zu) is not divisible by source stride (%zu)",
                        copy_size, src_stride);
                return;
            }

            if (dest_offset + copy_length > dest_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "copied area (offset = %zu, length = %zu) is out of destination bounds (length = %zu)",
                        dest_offset, copy_length, dest_length);
                return;
            }
            else if (src_offset + (copy_size / src_stride) > src_length)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "copied area (offset = %zu, length = %zu) is out of source bounds (length = %zu)",
                        src_offset, copy_size / src_stride, src_length);
                return;
            }

            // Copy the memory
            memmove((char*)context->ptr + dest_offset * dest_stride,
                    (const char*)source.cptr + src_offset * src_stride,
                    copy_size);

            ARCHI_ERROR_RESET();
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
            archi_pointer_attr_unpk__pdata(context->attr, &length, &stride, &alignment, NULL);

            // Parse parameters
            archi_rcpointer_t pattern = {0};
            size_t offset = 0, fill_length = 0;
            bool pattern_set = false, fill_length_set = false;
            {
                archi_plist_param_t parsed[] = {
                    {.name = "pattern",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__pdata(1, stride, alignment, NULL)}},
                        .assign = {archi_plist_assign__rcpointer, &pattern, sizeof(pattern), &pattern_set}},
                    {.name = "offset",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &offset, sizeof(offset)}},
                    {.name = "length",
                        .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                        .assign = {archi_plist_assign__value, &fill_length, sizeof(fill_length), &fill_length_set}},
                    {0},
                };

                if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
                    return;
            }

            // Check validness of parameters
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
            archi_pointer_attr_unpk__pdata(pattern.attr, &pattern_length, NULL, NULL, NULL);

            if (fill_length % pattern_length != 0)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "filled destination length (%zu) is not divisible by pattern length (%zu)",
                        fill_length, pattern_length);
                return;
            }

            // Fill the memory
            size_t num_patterns = fill_length / pattern_length;
            size_t pattern_size = pattern_length * stride;
            for (size_t i = 0; i < num_patterns; i++)
                memcpy((char*)context->ptr + (offset + pattern_length * i) * stride,
                        pattern.cptr, pattern_size);

            ARCHI_ERROR_RESET();
        }
        else
            ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
    }
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__pdpointer)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

    if (ARCHI_STRING_COMPARE("pointee", ==, slot.name))
    {
        if (slot.num_indices != 0)
        {
            ARCHI_ERROR_SET(ARCHI__EINDEX, "number of slot indices isn't 0");
            return;
        }
        else if (!archi_pointer_attr_unpk__pdata(value.attr, NULL, NULL, NULL, NULL))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not primitive data");
            return;
        }

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAM);
        if (!value.attr)
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
                    ARCHI_POINTER_ATTR__PDATA(1, char)))
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
                    ARCHI_POINTER_ATTR__PDATA(1, size_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a size_t");
            return;
        }

        size_t length = *(size_t*)value.ptr;

        size_t stride, alignment;
        archi_pointer_attr_unpk__pdata(context->attr, NULL, &stride, &alignment, NULL);

        archi_pointer_attr_t attr = archi_pointer_attr__pdata(length, stride, alignment,
                ARCHI_ERROR_PARAM);
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
                    ARCHI_POINTER_ATTR__PDATA(1, size_t)))
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
            archi_pointer_attr_unpk__pdata(context->attr, &length, &old_stride, &alignment, NULL);

            size_t size = length * old_stride;
            if (size % stride != 0)
            {
                ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data size (%zu) is not divisible by new stride (%zu)",
                        size, stride);
                return;
            }

            length = size / stride;
        }

        archi_pointer_attr_t attr = archi_pointer_attr__pdata(length, stride, alignment,
                ARCHI_ERROR_PARAM);
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
                    ARCHI_POINTER_ATTR__PDATA(1, size_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not a size_t");
            return;
        }

        size_t alignment = *(size_t*)value.ptr;

        size_t length, stride;
        archi_pointer_attr_unpk__pdata(context->attr, &length, &stride, NULL, NULL);

        archi_pointer_attr_t attr = archi_pointer_attr__pdata(length, stride, alignment,
                ARCHI_ERROR_PARAM);
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
archi_context_interface__pdpointer = {
    .init_fn = archi_context_init__pdpointer,
    .final_fn = archi_context_final__pointer,
    .eval_fn = archi_context_eval__pdpointer,
    .set_fn = archi_context_set__pdpointer,
};

/*****************************************************************************/

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__cdpointer)
{
    // Parse parameters
    archi_rcpointer_t data = {0};
    long long offset = 0;
    size_t offset_unit = 1;
    bool writable = false;
    archi_pointer_attr_t tag = 0;
    bool writable_set = false, tag_set = false;
    {
        archi_plist_param_t parsed[] = {
            {.name = "pointee",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__cdata(0)}},
                .assign = {archi_plist_assign__rcpointer, &data, sizeof(data)}},
            {.name = "offset",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, long long)}},
                .assign = {archi_plist_assign__value, &offset, sizeof(offset)}},
            {.name = "offset_unit",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, size_t)}},
                .assign = {archi_plist_assign__value, &offset_unit, sizeof(offset_unit)}},
            {.name = "writable",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, char)}},
                .assign = {archi_plist_assign__bool, &writable, sizeof(writable), &writable_set}},
            {.name = "tag",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_pointer_attr_t)}},
                .assign = {archi_plist_assign__value, &tag, sizeof(tag), &tag_set}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Check validness of parameters
    if (offset_unit == 0)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointer offset unit is zero");
        return NULL;
    }
    else if (tag > ARCHI_POINTER_DATA_TAG_MAX)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data type tag (%llu) exceeds ARCHI_POINTER_DATA_TAG_MAX",
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
    *context_data = archi_rcpointer_own(data, ARCHI_ERROR_PARAM);
    if (!context_data->attr)
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
    if (tag_set || !archi_pointer_attr_unpk__cdata(context_data->attr, NULL, NULL))
        context_data->attr = (context_data->attr & ARCHI_POINTER_TYPE_MASK) | archi_pointer_attr__cdata(tag);

    ARCHI_ERROR_RESET();
    return context_data;
}

static
ARCHI_CONTEXT_EVAL_FUNC(archi_context_eval__cdpointer)
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
                ARCHI_POINTER_ATTR__PDATA(1, char),
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
        if (!archi_pointer_attr_unpk__cdata(context->attr, &tag, ARCHI_ERROR_PARAM))
            return;

        archi_rcpointer_t value = {
            .ptr = &tag,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, archi_pointer_attr_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__cdpointer)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

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

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAM);
        if (!value.attr)
            return;

        if (!archi_pointer_attr_unpk__cdata(value.attr, NULL, NULL))
            value.attr = (value.attr & ARCHI_POINTER_TYPE_MASK) | archi_pointer_attr__cdata(0);

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
                    ARCHI_POINTER_ATTR__PDATA(1, char)))
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
                    ARCHI_POINTER_ATTR__PDATA(1, archi_pointer_attr_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not pointer attributes");
            return;
        }

        archi_pointer_attr_t tag = *(archi_pointer_attr_t*)value.ptr;
        if (tag > ARCHI_POINTER_DATA_TAG_MAX)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "data type tag (%llu) exceeds ARCHI_POINTER_DATA_TAG_MAX",
                    (unsigned long long)tag);
            return;
        }

        context->attr = (context->attr & ARCHI_POINTER_TYPE_MASK) | archi_pointer_attr__cdata(tag);
    }
    else
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
        return;
    }

    ARCHI_ERROR_RESET();
}

const archi_context_interface_t
archi_context_interface__cdpointer = {
    .init_fn = archi_context_init__cdpointer,
    .final_fn = archi_context_final__pointer,
    .eval_fn = archi_context_eval__cdpointer,
    .set_fn = archi_context_set__cdpointer,
};

/*****************************************************************************/

static
ARCHI_CONTEXT_INIT_FUNC(archi_context_init__fpointer)
{
    // Parse parameters
    archi_rcpointer_t function = {.attr = archi_pointer_attr__func(0)};
    archi_pointer_attr_t tag = 0;
    bool tag_set = false;
    {
        archi_plist_param_t parsed[] = {
            {.name = "pointee",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){archi_pointer_attr__func(0)}},
                .assign = {archi_plist_assign__rcpointer, &function, sizeof(function)}},
            {.name = "tag",
                .check = {archi_value_check__attr, (archi_pointer_attr_t[]){ARCHI_POINTER_ATTR__PDATA(1, archi_pointer_attr_t)}},
                .assign = {archi_plist_assign__value, &tag, sizeof(tag), &tag_set}},
            {0},
        };

        if (!archi_plist_parse(&params->n, true, parsed, false, ARCHI_ERROR_PARAM))
            return NULL;
    }

    // Check validness of parameters
    if (tag > ARCHI_POINTER_FUNC_TAG_MAX)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "function type tag (%llu) exceeds ARCHI_POINTER_FUNC_TAG_MAX",
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
    *context_data = archi_rcpointer_own(function, ARCHI_ERROR_PARAM);
    if (!context_data->attr)
    {
        free(context_data);
        return NULL;
    }

    // Set function type tag
    if (tag_set)
        context_data->attr = archi_pointer_attr__func(tag);

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
        if (!archi_pointer_attr_unpk__func(context->attr, &tag, ARCHI_ERROR_PARAM))
            return;

        archi_rcpointer_t value = {
            .ptr = &tag,
            .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                ARCHI_POINTER_ATTR__PDATA(1, archi_pointer_attr_t),
        };

        ARCHI_CONTEXT_YIELD(value);
    }
    else
        ARCHI_ERROR_SET(ARCHI__EKEY, "unknown slot '%s' encountered", slot.name);
}

static
ARCHI_CONTEXT_SET_FUNC(archi_context_set__fpointer)
{
    if (unset)
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "slot unsetting is not supported");
        return;
    }

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

        value = archi_rcpointer_own_disown(value, *context, ARCHI_ERROR_PARAM);
        if (!value.attr)
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
                    ARCHI_POINTER_ATTR__PDATA(1, archi_pointer_attr_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "assigned value is not pointer attributes");
            return;
        }

        archi_pointer_attr_t tag = *(archi_pointer_attr_t*)value.ptr;
        if (tag > ARCHI_POINTER_FUNC_TAG_MAX)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "function type tag (%llu) exceeds ARCHI_POINTER_FUNC_TAG_MAX",
                    (unsigned long long)tag);
            return;
        }

        context->attr = archi_pointer_attr__func(tag);
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

