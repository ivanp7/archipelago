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

#include "archi_app/registry.fun.h"
#include "archi/context/api/registry.fun.h"
#include "archi/context/api/tag.def.h"
#include "archi/context/ctx/parameters.var.h"
#include "archi/context/ctx/pointer.var.h"
#include "archi/context/ctx/dptr_array.var.h"
#include "archi_base/kvlist.fun.h"
#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"
#include "archi_log/print.fun.h"


#define INDENTATION_STEP    4
#define INDENTATION         (INDENTATION_STEP * 1)
#define INDENTATION2        (INDENTATION_STEP * 3)

static
void
archi_print_key(
        const char *field,
        const char *key)
{
    archi_print("%*s%s = ", INDENTATION, "", field);
    {
        if (key != NULL)
            archi_print("\"%s\"", key);
        else
            archi_print("NULL");

        archi_print("\n");
    }
}

static
void
archi_print_size(
        const char *field,
        size_t size)
{
    archi_print("%*s%s = ", INDENTATION, "", field);
    {
        archi_print("%zu", size);

        archi_print("\n");
    }
}

static
void
archi_print_slot(
        const char *field,
        archi_context_slot_t slot)
{
    archi_print("%*s%s.name = ", INDENTATION, "", field);
    {
        if (slot.name != NULL)
            archi_print("\"%s\"", slot.name);
        else
            archi_print("NULL");

        archi_print("\n");
    }

    archi_print("%*s%s.index[%zu] =", INDENTATION, "", field, slot.num_indices);
    {
        if (slot.index != NULL)
            for (size_t i = 0; i < slot.num_indices; i++)
                archi_print(" %lli", slot.index[i]);
        else
            archi_print(" NULL");

        archi_print("\n");
    }
}

static
bool
archi_print_pointer(
        const char *field,
        archi_pointer_t value)
{
    bool viewable = false;

    archi_print("%*s%s = ", INDENTATION, "", field);
    {
        if (!archi_pointer_valid(value, NULL))
            archi_print("(INVALID) ");

        if (value.ptr != NULL)
            archi_print("%p", value.ptr);
        else
            archi_print("NULL");

        archi_print(" <");

        size_t length, stride, alignment;
        archi_pointer_attr_t tag;

        if (archi_pointer_attr_unpk__pdata(value.attr, &length, &stride, &alignment, NULL))
        {
            const char *mem_type;

            if (ARCHI_POINTER_TO_READONLY_DATA(value.attr))
                mem_type = "read-only";
            else if (ARCHI_POINTER_TO_WRITABLE_DATA(value.attr))
                mem_type = "writable";
            else
                mem_type = "on stack";

            archi_print("data, %s, length = %zu, stride = %zu, alignment = %#zx",
                    mem_type, length, stride, alignment);

            viewable = true;
        }
        else if (archi_pointer_attr_unpk__cdata(value.attr, &tag, NULL))
        {
            const char *mem_type;

            if (ARCHI_POINTER_TO_READONLY_DATA(value.attr))
                mem_type = "read-only";
            else if (ARCHI_POINTER_TO_WRITABLE_DATA(value.attr))
                mem_type = "writable";
            else
                mem_type = "on stack";

            archi_print("data, %s, tag = %llu",
                    mem_type, (unsigned long long)tag);
        }
        else if (archi_pointer_attr_unpk__func(value.attr, &tag, NULL))
        {
            archi_print("function, tag = %llu",
                    (unsigned long long)tag);
        }
        else // must never happen
        {
            archi_print("unknown");
        }

        archi_print(">\n");
    }

    return viewable;
}

static
void
archi_print_pointer_with_contents(
        const char *field,
        archi_pointer_t value)
{
    if (!archi_print_pointer(field, value))
        return;

    size_t length, stride;
    archi_pointer_attr_unpk__pdata(value.attr, &length, &stride, NULL, NULL);
    size_t size = length * stride;

    if ((value.ptr == NULL) || (length == 0))
        return;

#define PRINT_WIDTH             80
#define PRINT_AS(type, format)  do {                        \
        archi_print("%*sas " #type ":", INDENTATION2, "");  \
        int printed_chars = 0;                              \
        for (size_t index = 0; index < length; index++) {   \
            if (printed_chars >= PRINT_WIDTH) {             \
                archi_print(" ...");                        \
                break;                                      \
            }                                               \
            type element = ((type*)value.ptr)[index];       \
            int p = archi_print(" " format, element);       \
            if (p < 0) break;                               \
            printed_chars += p;                             \
        }                                                   \
        archi_print("\n");                                  \
    } while (0)

    if (stride == sizeof(char))
    {
        PRINT_AS(signed char, "%hhi");
        PRINT_AS(unsigned char, "%hhu");
    }
    else if (stride == sizeof(short))
    {
        PRINT_AS(signed short, "%hi");
        PRINT_AS(unsigned short, "%hu");
    }
    else if (stride == sizeof(int))
    {
        PRINT_AS(signed int, "%i");
        PRINT_AS(unsigned int, "%u");
    }
    else if (stride == sizeof(long))
    {
        PRINT_AS(signed long, "%li");
        PRINT_AS(unsigned long, "%lu");
    }
    else if (stride == sizeof(long long))
    {
        PRINT_AS(signed long long, "%lli");
        PRINT_AS(unsigned long long, "%llu");
    }

    if (stride == sizeof(float))
    {
        PRINT_AS(float, "%g");
    }
    else if (stride == sizeof(double))
    {
        PRINT_AS(double, "%g");
    }
    else if (stride == sizeof(long double))
    {
        PRINT_AS(long double, "%Lg");
    }

#undef PRINT_AS
#undef PRINT_WIDTH

#define MAX_SIZE    (16*4) // 4 lines of 16 bytes each

    if (size > MAX_SIZE)
        size = MAX_SIZE;

    archi_print_hex_dump(value.ptr, size, false, INDENTATION2);

#undef MAX_SIZE
}

static
void
archi_print_params(
        const char *field,
        const archi_kvlist_t *params)
{
    archi_print("%*s%s:", INDENTATION, "", field);
    {
        if (params != NULL)
        {
            archi_print("\n");

            for (; params != NULL; params = params->next)
            {
                archi_print("%*s", INDENTATION_STEP, "");
                archi_print_pointer_with_contents(params->key, params->value);
            }
        }
        else
            archi_print(" NULL\n");
    }
}

#undef INDENTATION

/*****************************************************************************/

#define OPERATION_PREAMBLE(data_type)                                                                   \
    (void) ref_count;                                                                                   \
    if (!archi_pointer_attr_compatible(data.attr, ARCHI_POINTER_ATTR__PDATA(1, data_type))) {       \
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "registry operation data has incorrect type"); return; }    \
    else if (data.ptr == NULL) {                                                                        \
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "registry operation data pointer is NULL"); return; }       \
    else if (!archi_pointer_valid(data, NULL)) {                                                        \
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "registry operation data pointer is invalid"); return; }    \
    const data_type *op_data = data.ptr;

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__delete)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__delete_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);

        ARCHI_ERROR_RESET();
        return;
    }

    // Do the operation
    archi_context_registry_delete(registry, op_data->key,
            ARCHI_ERROR_PARAM);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__alias)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__alias_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_key("original_key", op_data->original_key);

        ARCHI_ERROR_RESET();
        return;
    }

    if (!archi_context_registry_key_available(registry, op_data->key))
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "context key '%s' is used already", op_data->key);
        return;
    }

    // Do the operation
    archi_context_registry_alias(registry, op_data->key, op_data->original_key,
            ARCHI_ERROR_PARAM);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_as)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__create_as_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_key("sample_key", op_data->sample_key);
        archi_print_key("init_params.context_key", op_data->init_params.context_key);
        archi_print_params("init_params.list", op_data->init_params.list);

        ARCHI_ERROR_RESET();
        return;
    }

    if (!archi_context_registry_key_available(registry, op_data->key))
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "context key '%s' is used already", op_data->key);
        return;
    }

    // Copy the parameter list
    archi_context_registry_params_t init_params = {
        .context_key = op_data->init_params.context_key,
        .list = (archi_krcvlist_t*)archi_kvlist_copy(op_data->init_params.list, true, ref_count, NULL),
    };

    if ((op_data->init_params.list != NULL) && (init_params.list == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate parameter list copy");
        return;
    }

    // Do the operation
    archi_context_registry_create_as(registry, op_data->key, op_data->sample_key, init_params,
            ARCHI_ERROR_PARAM);

    // Free the parameter list copy
    archi_krcvlist_free(init_params.list, false);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_from)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__create_from_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_key("source_key", op_data->source_key);
        archi_print_slot("source_slot", op_data->source_slot);
        archi_print_key("init_params.context_key", op_data->init_params.context_key);
        archi_print_params("init_params.list", op_data->init_params.list);

        ARCHI_ERROR_RESET();
        return;
    }

    if (!archi_context_registry_key_available(registry, op_data->key))
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "context key '%s' is used already", op_data->key);
        return;
    }

    // Copy the parameter list
    archi_context_registry_params_t init_params = {
        .context_key = op_data->init_params.context_key,
        .list = (archi_krcvlist_t*)archi_kvlist_copy(op_data->init_params.list, true, ref_count, NULL),
    };

    if ((op_data->init_params.list != NULL) && (init_params.list == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate parameter list copy");
        return;
    }

    // Do the operation
    archi_context_registry_create_from(registry, op_data->key,
            op_data->source_key, op_data->source_slot, init_params,
            ARCHI_ERROR_PARAM);

    // Free the parameter list copy
    archi_krcvlist_free(init_params.list, false);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_params)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__create_params_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_key("params.context_key", op_data->params.context_key);
        archi_print_params("params.list", op_data->params.list);

        ARCHI_ERROR_RESET();
        return;
    }

    if (!archi_context_registry_key_available(registry, op_data->key))
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "context key '%s' is used already", op_data->key);
        return;
    }

    // Copy the parameter list
    archi_context_registry_params_t params = {
        .context_key = op_data->params.context_key,
        .list = (archi_krcvlist_t*)archi_kvlist_copy(op_data->params.list, true, ref_count, NULL),
    };

    if ((op_data->params.list != NULL) && (params.list == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate parameter list copy");
        return;
    }

    // Prepare the interface
    archi_rcpointer_t interface = {
        .cptr = &archi_context_interface__parameters,
        .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE),
    };

    // Do the operation
    archi_context_registry_create(registry, op_data->key, interface, params,
            ARCHI_ERROR_PARAM);

    // Free the parameter list copy
    archi_krcvlist_free(params.list, false);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_ptr)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__create_ptr_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_pointer_with_contents("pointee", op_data->pointee);

        ARCHI_ERROR_RESET();
        return;
    }

    if (!archi_context_registry_key_available(registry, op_data->key))
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "context key '%s' is used already", op_data->key);
        return;
    }

    // Prepare the interface
    archi_rcpointer_t interface = {
        .cptr = &archi_context_interface__pointer,
        .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE),
    };

    // Do the operation
    archi_krcvlist_t params[] = {
        {
            .key = "pointee",
            .value = archi_rcpointer(op_data->pointee, ref_count),
        },
    };

    archi_context_registry_create(registry, op_data->key, interface,
            (archi_context_registry_params_t){.list = params},
            ARCHI_ERROR_PARAM);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__create_dptr_array)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__create_dptr_array_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_size("length", op_data->length);

        ARCHI_ERROR_RESET();
        return;
    }

    if (!archi_context_registry_key_available(registry, op_data->key))
    {
        ARCHI_ERROR_SET(ARCHI__EKEY, "context key '%s' is used already", op_data->key);
        return;
    }

    // Prepare the interface
    archi_rcpointer_t interface = {
        .cptr = &archi_context_interface__dptr_array,
        .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
            archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG__CONTEXT_INTERFACE),
    };

    // Do the operation
    archi_krcvlist_t params[] = {
        {
            .key = "length",
            .value = {
                .cptr = &op_data->length,
                .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                    ARCHI_POINTER_ATTR__PDATA(1, size_t),
                .ref_count = ref_count,
            },
        },
    };

    archi_context_registry_create(registry, op_data->key, interface,
            (archi_context_registry_params_t){.list = params},
            ARCHI_ERROR_PARAM);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__invoke)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__invoke_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_slot("slot", op_data->slot);
        archi_print_key("call_params.context_key", op_data->call_params.context_key);
        archi_print_params("call_params.list", op_data->call_params.list);

        ARCHI_ERROR_RESET();
        return;
    }

    // Copy the parameter list
    archi_context_registry_params_t call_params = {
        .context_key = op_data->call_params.context_key,
        .list = (archi_krcvlist_t*)archi_kvlist_copy(op_data->call_params.list, true, ref_count, NULL),
    };

    if ((op_data->call_params.list != NULL) && (call_params.list == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate parameter list copy");
        return;
    }

    // Do the operation
    archi_context_registry_invoke(registry, op_data->key, op_data->slot, call_params,
            ARCHI_ERROR_PARAM);

    // Free the parameter list copy
    archi_krcvlist_free(call_params.list, false);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__unassign)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__unassign_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_slot("slot", op_data->slot);

        ARCHI_ERROR_RESET();
        return;
    }

    // Do the operation
    archi_context_registry_unassign(registry, op_data->key, op_data->slot,
            ARCHI_ERROR_PARAM);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__assign_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_slot("slot", op_data->slot);
        archi_print_pointer_with_contents("value", op_data->value);

        ARCHI_ERROR_RESET();
        return;
    }

    // Do the operation
    archi_context_registry_assign(registry, op_data->key, op_data->slot, false,
            archi_rcpointer(op_data->value, ref_count),
            ARCHI_ERROR_PARAM);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign_slot)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__assign_slot_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_slot("slot", op_data->slot);
        archi_print_key("source_key", op_data->source_key);
        archi_print_slot("source_slot", op_data->source_slot);

        ARCHI_ERROR_RESET();
        return;
    }

    // Do the operation
    archi_context_registry_assign_slot(registry, op_data->key, op_data->slot, false,
            op_data->source_key, op_data->source_slot,
            ARCHI_ERROR_PARAM);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign_slot_weak)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__assign_slot_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_slot("slot", op_data->slot);
        archi_print_key("source_key", op_data->source_key);
        archi_print_slot("source_slot", op_data->source_slot);

        ARCHI_ERROR_RESET();
        return;
    }

    // Do the operation
    archi_context_registry_assign_slot(registry, op_data->key, op_data->slot, true,
            op_data->source_key, op_data->source_slot,
            ARCHI_ERROR_PARAM);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign_call)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__assign_call_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_slot("slot", op_data->slot);
        archi_print_key("source_key", op_data->source_key);
        archi_print_slot("source_slot", op_data->source_slot);
        archi_print_key("source_call_params.context_key", op_data->source_call_params.context_key);
        archi_print_params("source_call_params.list", op_data->source_call_params.list);

        ARCHI_ERROR_RESET();
        return;
    }

    // Copy the parameter list
    archi_context_registry_params_t source_call_params = {
        .context_key = op_data->source_call_params.context_key,
        .list = (archi_krcvlist_t*)archi_kvlist_copy(op_data->source_call_params.list, true, ref_count, NULL),
    };

    if ((op_data->source_call_params.list != NULL) && (source_call_params.list == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate parameter list copy");
        return;
    }

    // Do the operation
    archi_context_registry_assign_call(registry, op_data->key, op_data->slot, false,
            op_data->source_key, op_data->source_slot, source_call_params,
            ARCHI_ERROR_PARAM);

    // Free the parameter list copy
    archi_krcvlist_free(source_call_params.list, false);
}

ARCHI_APP_REGISTRY_OPERATION_FUNC(archi_app_registry_op__assign_call_weak)
{
    OPERATION_PREAMBLE(archi_app_registry_op_data__assign_call_t);

    if (registry == NULL)
    {
        // Print operation data and return
        archi_print_key("key", op_data->key);
        archi_print_slot("slot", op_data->slot);
        archi_print_key("source_key", op_data->source_key);
        archi_print_slot("source_slot", op_data->source_slot);
        archi_print_key("source_call_params.context_key", op_data->source_call_params.context_key);
        archi_print_params("source_call_params.list", op_data->source_call_params.list);

        ARCHI_ERROR_RESET();
        return;
    }

    // Copy the parameter list
    archi_context_registry_params_t source_call_params = {
        .context_key = op_data->source_call_params.context_key,
        .list = (archi_krcvlist_t*)archi_kvlist_copy(op_data->source_call_params.list, true, ref_count, NULL),
    };

    if ((op_data->source_call_params.list != NULL) && (source_call_params.list == NULL))
    {
        ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate parameter list copy");
        return;
    }

    // Do the operation
    archi_context_registry_assign_call(registry, op_data->key, op_data->slot, true,
            op_data->source_key, op_data->source_slot, source_call_params,
            ARCHI_ERROR_PARAM);

    // Free the parameter list copy
    archi_krcvlist_free(source_call_params.list, false);
}

