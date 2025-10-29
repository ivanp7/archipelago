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
 *   You should have received a assign of the GNU Lesser General Public        *
 *   License along with Archipelago. If not, see                             *
 *   <http://www.gnu.org/licenses/>.                                         *
 *****************************************************************************/

/**
 * @file
 * @brief Context registry operations.
 */

#include "archi/registry/ctx-op/registry.fun.h"
#include "archi/registry/ctx-op/registry.typ.h"
#include "archi/context/ctx/parameters.var.h"
#include "archi/context/ctx/pointer.var.h"
#include "archi/context/ctx/dptr_array.var.h"
#include "archi/context/api/interface.fun.h"
#include "archi/context/api/callback.fun.h"
#include "archipelago/base/pointer.fun.h"
#include "archipelago/base/pointer.def.h"
#include "archipelago/base/kvlist.fun.h"
#include "archipelago/base/ref_count.fun.h"
#include "archipelago/log/print.fun.h"

#define LOG_INDENT "    "

static
void
archi_print_memory(
        const char *ptr,
        size_t length,
        size_t stride)
{
#define MAX_BYTES       (1 << 5)

#define MAX_CHARS       (MAX_BYTES / sizeof(char))
#define MAX_SHORTS      (MAX_BYTES / sizeof(short))
#define MAX_INTS        (MAX_BYTES / sizeof(int))
#define MAX_LONGS       (MAX_BYTES / sizeof(long))
#define MAX_LONGLONGS   (MAX_BYTES / sizeof(long long))
#define MAX_FLOATS      (MAX_BYTES / sizeof(float))
#define MAX_DOUBLES     (MAX_BYTES / sizeof(double))
#define MAX_LONGDOUBLES (MAX_BYTES / sizeof(long double))

    static const char *indent = LOG_INDENT LOG_INDENT;

    // Print bytes
    {
        archi_print("%s  bytes:", indent);

        for (size_t i = 0; i < MAX_BYTES; i++)
        {
            if (i / stride >= length)
                break;

            unsigned char val;
            memcpy(&val, ptr + i, sizeof(char));

            archi_print(" %02x", (int)val);
        }

        archi_print("\n");
    }

    // Print data elements of the correct size
#define archi_print_ELEMENTS(max_elements, type, name, format) do { \
        archi_print("%s  " name ":", indent);                       \
        for (size_t i = 0; i < (max_elements); i++)                 \
        {                                                           \
            if (i >= length)                                        \
                break;                                              \
            type val;                                               \
            memcpy(&val, ptr + i * stride, sizeof(type));           \
            archi_print(" " format, val);                           \
        }                                                           \
        archi_print("\n");                                          \
    } while (0)

    if (stride == sizeof(char))
    {
        // Print string
        archi_print("%s  string: ", indent);
        for (size_t i = 0; i < MAX_CHARS; i++)
        {
            if (i >= length)
                break;

            unsigned char val;
            memcpy(&val, ptr + i * stride, sizeof(char));

            if (val >= 32)
                archi_print("%c", (int)val);
            else
                archi_print(".");
        }
        archi_print("\n");

        archi_print_ELEMENTS(MAX_CHARS, unsigned char, "uchar", "%hhu");
        archi_print_ELEMENTS(MAX_CHARS, signed char, "schar", "%hhi");
    }
    else if (stride == sizeof(short))
    {
        archi_print_ELEMENTS(MAX_SHORTS, unsigned short, "ushort", "%hu");
        archi_print_ELEMENTS(MAX_SHORTS, signed char, "sshort", "%hi");
    }
    else if (stride == sizeof(int))
    {
        archi_print_ELEMENTS(MAX_INTS, unsigned int, "uint", "%u");
        archi_print_ELEMENTS(MAX_INTS, signed int, "sint", "%i");
    }
    else if (stride == sizeof(long))
    {
        archi_print_ELEMENTS(MAX_LONGS, unsigned long, "ulong", "%lu");
        archi_print_ELEMENTS(MAX_LONGS, signed long, "slong", "%li");
    }
    else if (stride == sizeof(long long))
    {
        archi_print_ELEMENTS(MAX_LONGLONGS, unsigned long long, "ulonglong", "%llu");
        archi_print_ELEMENTS(MAX_LONGLONGS, signed long long, "slonglong", "%lli");
    }

    if (stride == sizeof(float))
    {
        archi_print_ELEMENTS(MAX_FLOATS, float, "float", "%e");
    }
    else if (stride == sizeof(double))
    {
        archi_print_ELEMENTS(MAX_DOUBLES, double, "double", "%e");
    }
    else if (stride == sizeof(long double))
    {
        archi_print_ELEMENTS(MAX_LONGDOUBLES, long double, "longdouble", "%Le");
    }

#undef archi_print_ELEMENTS

#undef MAX_BYTES

#undef MAX_CHARS
#undef MAX_SHORTS
#undef MAX_INTS
#undef MAX_LONGS
#undef MAX_LONGLONGS
#undef MAX_FLOATS
#undef MAX_DOUBLES
#undef MAX_LONGDOUBLES
}

static
void
archi_print_value(
        const char *name,
        archi_pointer_t value)
{
    size_t length, stride, alignment;
    archi_pointer_attr_t tag;

    if (archi_pointer_attr_parse__transp_data(value.attr, &length, &stride, &alignment, NULL))
    {
        const char *mem_type;

        if (ARCHI_POINTER_TO_READONLY_DATA(value.attr))
            mem_type = "read-only";
        else if (ARCHI_POINTER_TO_WRITABLE_DATA(value.attr))
            mem_type = "writable";
        else
            mem_type = "on stack";

        archi_print(LOG_INDENT "%s = <data, %s, transparent: length = %zu, stride = %zu, alignment = %#zx>:\n",
                name, mem_type, length, stride, alignment);

        archi_print_memory(value.ptr, length, stride);
    }
    else if (archi_pointer_attr_parse__opaque_data(value.attr, &tag, NULL))
    {
        const char *mem_type;

        if (ARCHI_POINTER_TO_READONLY_DATA(value.attr))
            mem_type = "read-only";
        else if (ARCHI_POINTER_TO_WRITABLE_DATA(value.attr))
            mem_type = "writable";
        else
            mem_type = "on stack";

        archi_print(LOG_INDENT "%s = <data, %s, opaque: tag = %llu>\n", name, mem_type, (unsigned long long)tag);
    }
    else if (archi_pointer_attr_parse__function(value.attr, &tag, NULL))
    {
        archi_print(LOG_INDENT "%s = <function: tag = %llu>\n", name, (unsigned long long)tag);
    }
    else // must never happen
    {
        archi_print(LOG_INDENT "%s = <unknown>\n", name);
    }
}

static
void
archi_print_key(
        const char *name,
        const char *value)
{
    archi_print(LOG_INDENT "%s = ", name);

    if (value != NULL)
        archi_print("\"%s\"\n", value);
    else
        archi_print("NULL\n");
}

static
void
archi_print_slot(
        const char *name,
        archi_context_slot_t slot)
{
    archi_print(LOG_INDENT "%s.name = ", name);

    if (slot.name != NULL)
        archi_print("\"%s\"\n", slot.name);
    else
        archi_print("NULL\n");

    archi_print(LOG_INDENT "%s.indices[%zu] =", name, slot.num_indices);

    for (size_t i = 0; i < slot.num_indices; i++)
        archi_print(" %ti", slot.index[i]);

    archi_print("\n");
}

static
void
archi_print_params(
        const char *name,
        const archi_kvlist_t *params)
{
    if (params != NULL)
    {
        archi_print(LOG_INDENT "%s:\n", name);

        for (; params != NULL; params = params->next)
        {
            archi_print("  ");
            archi_print_value(params->key, params->value);
        }
    }
    else
        archi_print(LOG_INDENT "%s: <none>\n", name);
}

static
void
archi_print_pointer_type(
        const char *name,
        archi_context_registry_op_data_ptr_type_t type)
{
    archi_print(LOG_INDENT "%s = ", name);

    switch (type)
    {
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__GENERIC:
            archi_print("<generic pointer>");
            break;
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__DATA:
            archi_print("<pointer to data>");
            break;
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__TRANSP_DATA:
            archi_print("<pointer to transparent data>");
            break;
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__OPAQUE_DATA:
            archi_print("<pointer to opaque data>");
            break;
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__FUNCTION:
            archi_print("<pointer to function>");
            break;
        default:
            archi_print("<unknown>");
    }

    archi_print("\n");
}

#undef LOG_INDENT

/*****************************************************************************/

static
archi_rcpointer_t
archi_context_registry_get(
        archi_context_t registry,
        const char *key,
        ARCHI_ERROR_PARAMETER_DECL)
{
    archi_rcpointer_t context = {0};
    archi_context_get(registry, (archi_context_slot_t){.name = key},
            (archi_context_callback_t){.function = archi_context_callback__getter,
            .data = &context}, ARCHI_ERROR_PARAMETER);

    if (!archi_pointer_attr_compatible(context.attr,
                archi_pointer_attr__opaque_data(ARCHI_POINTER_DATA_TAG__CONTEXT)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "object stored in context registry is not a context");
        return (archi_rcpointer_t){0};
    }

    return context;
}

struct archi_context_params_info {
    const archi_kvlist_rc_t *params; ///< Concatenated parameter list.

    archi_kvlist_rc_t *sparams_head; ///< Head node of the static parameter list copy.
    archi_kvlist_rc_t *sparams_tail; ///< Tail node of the static parameter list copy.
};

static
struct archi_context_params_info
archi_context_registry_init_params(
        archi_context_t registry,
        const char *dparams_key,
        const archi_kvlist_t *sparams,
        archi_reference_count_t sparams_ref_count,
        ARCHI_ERROR_PARAMETER_DECL)
{
    struct archi_context_params_info params_info = {0};

    // Obtain the dynamic parameter list
    archi_kvlist_rc_t *dparams = NULL;

    if (dparams_key != NULL)
    {
        // Obtain the parameter list context
        archi_error_t error = {0};
        archi_rcpointer_t dparams_context = archi_context_registry_get(registry, dparams_key, &error);
        ARCHI_ERROR_ASSIGN(error);

        if (error.code != 0)
            return params_info;

        archi_rcpointer_t dparams_ptr = archi_context_data(dparams_context.ptr);
        if (!archi_pointer_attr_compatible(dparams_ptr.attr,
                    ARCHI_POINTER_ATTR__DATA_TYPE(0, archi_kvlist_rc_t)))
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "dynamic parameter list context is not a refcounted key-value list");
            return params_info;
        }

        dparams = dparams_ptr.ptr;
    }

    // Copy the static parameter list
    archi_kvlist_rc_t *sparams_head, *sparams_tail;
    sparams_head = archi_kvlist_copy_with_refcounts(sparams, sparams_ref_count, &sparams_tail);

    if (sparams != NULL)
    {
        if (sparams_tail == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__EMEMORY, "couldn't allocate static parameter list copy");
            return params_info;
        }

        // Append dynamic list to the static list copy
        sparams_tail->next = dparams;
    }

    // Prepare the parameter list
    params_info.params = (sparams_head != NULL) ? sparams_head : dparams;
    params_info.sparams_head = sparams_head;
    params_info.sparams_tail = sparams_tail;

    return params_info;
}

static
void
archi_context_registry_final_params(
        struct archi_context_params_info params_info)
{
    // Remove the dynamic list from the static list copy
    if (params_info.sparams_tail != NULL)
        params_info.sparams_tail->next = NULL;

    archi_kvlist_rc_free(params_info.sparams_head, false);
}

static
void
archi_context_registry_add(
        archi_context_t registry,
        const char *key,
        archi_rcpointer_t interface_ptr,

        const char *dparams_key,
        const archi_kvlist_t *sparams,
        archi_reference_count_t sparams_ref_count,

        archi_kvlist_rc_t hardcoded_params[],

        ARCHI_ERROR_PARAMETER_DECL)
{
    archi_context_t context;
    {
        // Prepare the parameter list
        archi_error_t error = {0};
        struct archi_context_params_info params_info =
            archi_context_registry_init_params(registry,
                    dparams_key, sparams, sparams_ref_count, &error);
        ARCHI_ERROR_ASSIGN(error);

        if (error.code != 0)
            return;

        const archi_kvlist_rc_t *params = params_info.params;

        archi_kvlist_rc_t *hardcoded_params_tail =
            (archi_kvlist_rc_t*)archi_kvlist_rc_tail(hardcoded_params);
        if (hardcoded_params != NULL)
        {
            hardcoded_params_tail->next = (archi_kvlist_rc_t*)params_info.params;
            params = hardcoded_params;
        }

        // Initialize the context
        context = archi_context_initialize(interface_ptr, params, ARCHI_ERROR_PARAMETER);

        // Finalize the parameter list
        if (hardcoded_params_tail != NULL)
            hardcoded_params_tail->next = NULL;
        archi_context_registry_final_params(params_info);
    }

    // Insert the context to the registry
    if (context == NULL)
        return;

    archi_rcpointer_t context_ptr = {
        .ptr = context,
        .attr = ARCHI_POINTER_TYPE__DATA_WRITABLE |
            archi_pointer_attr__opaque_data(ARCHI_POINTER_DATA_TAG__CONTEXT),
        .ref_count = archi_context_data(context).ref_count, // the reference count is 1 at this point
    };

    archi_context_set(registry, (archi_context_slot_t){.name = key},
            context_ptr, ARCHI_ERROR_PARAMETER); // increments the reference count to 2

    // Decrement the reference count back to 1, making registry the exclusive owner of the context
    archi_reference_count_decrement(context_ptr.ref_count);
}

static
const archi_context_interface_t*
archi_context_interface_for_pointer_type(
        archi_context_registry_op_data_ptr_type_t type)
{
    switch (type)
    {
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__GENERIC:
            return &archi_context_interface__pointer;
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__DATA:
            return &archi_context_interface__dpointer;
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__TRANSP_DATA:
            return &archi_context_interface__tdpointer;
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__OPAQUE_DATA:
            return &archi_context_interface__odpointer;
        case ARCHI_CONTEXT_REGISTRY_PTR_TYPE__FUNCTION:
            return &archi_context_interface__fpointer;
        default:
            return NULL;
    }
}

/*****************************************************************************/

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__delete)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__delete_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__delete_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("key", op_data->key);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "deleted context key is NULL");
            return;
        }
        else if (op_data->key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "deleted context key is empty");
            return;
        }

        // Delete context from the registry
        archi_kvlist_rc_t params[] = {
            {
                .key = "unset",
                .value = {
                    .cptr = op_data->key,
                    .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                        ARCHI_POINTER_ATTR__DATA_TYPE(strlen(op_data->key) + 1, char),
                    .ref_count = argument.ref_count,
                },
            },
        };

        archi_context_call(context, (archi_context_slot_t){.name = ""}, params,
                (archi_context_callback_t){0}, ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__alias)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__alias_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__alias_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("alias_key", op_data->alias_key);
        archi_print_key("origin_key", op_data->origin_key);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->alias_key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "new context key is NULL");
            return;
        }
        else if (op_data->alias_key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "new context key is empty");
            return;
        }
        else if (op_data->origin_key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "old context key is NULL");
            return;
        }
        else if (op_data->origin_key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "old context key is empty");
            return;
        }

        // Assign the context to another key
        archi_context_set_from_get(context, (archi_context_slot_t){.name = op_data->alias_key},
                context, (archi_context_slot_t){.name = op_data->origin_key}, false,
                ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_as)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__create_as_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__create_as_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("context.key", op_data->context.key);
        archi_print_key("instance_key", op_data->instance_key);
        archi_print_key("context.init_params.list_key", op_data->context.init_params.list_key);
        archi_print_params("context.init_params.list", op_data->context.init_params.list);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->context.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is NULL");
            return;
        }
        else if (op_data->context.key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is empty");
            return;
        }
        else if (op_data->instance_key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "instance context key is NULL");
            return;
        }
        else if (op_data->instance_key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "instance context key is empty");
            return;
        }

        // Obtain the context interface
        archi_rcpointer_t interface_ptr;
        {
            // Obtain the instance context
            archi_error_t error = {0};
            archi_rcpointer_t instance = archi_context_registry_get(context, op_data->instance_key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;

            // Get interface of the instance context
            interface_ptr = archi_context_interface(instance.ptr);
        }

        // Initialize the context and add it to the registry
        archi_context_registry_add(context, op_data->context.key, interface_ptr,
                op_data->context.init_params.list_key, op_data->context.init_params.list,
                argument.ref_count, NULL, ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_from)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__create_from_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__create_from_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("context.key", op_data->context.key);
        archi_print_key("origin.key", op_data->origin.key);
        archi_print_slot("origin.slot", op_data->origin.slot);
        archi_print_key("context.init_params.list_key", op_data->context.init_params.list_key);
        archi_print_params("context.init_params.list", op_data->context.init_params.list);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->context.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is NULL");
            return;
        }
        else if (op_data->context.key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is empty");
            return;
        }
        else if (op_data->origin.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "interface origin context key is NULL");
            return;
        }
        else if (op_data->origin.key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "interface origin context key is empty");
            return;
        }

        // Obtain the context interface
        archi_rcpointer_t interface_ptr;
        {
            // Obtain the origin context
            archi_error_t error = {0};
            archi_rcpointer_t origin_context = archi_context_registry_get(context, op_data->origin.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;

            // Get interface from the origin context slot
            ARCHI_ERROR_RESET_VAR(&error);
            archi_context_get(origin_context.ptr, op_data->origin.slot,
                    (archi_context_callback_t){.function = archi_context_callback__getter,
                    .data = &interface_ptr}, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Initialize the context and add it to the registry
        archi_context_registry_add(context, op_data->context.key, interface_ptr,
                op_data->context.init_params.list_key, op_data->context.init_params.list,
                argument.ref_count, NULL, ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__call)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__call_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__call_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("target.key", op_data->target.key);
        archi_print_slot("target.slot", op_data->target.slot);
        archi_print_key("call_params.list_key", op_data->call_params.list_key);
        archi_print_params("call_params.list", op_data->call_params.list);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->target.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target context key is NULL");
            return;
        }

        // Obtain the target context
        archi_rcpointer_t target_context;
        {
            archi_error_t error = {0};
            target_context = archi_context_registry_get(context, op_data->target.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Prepare the parameter list
        struct archi_context_params_info params_info;
        {
            archi_error_t error = {0};
            params_info = archi_context_registry_init_params(context,
                    op_data->call_params.list_key, op_data->call_params.list,
                    argument.ref_count, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Invoke the target context call
        archi_context_call(target_context.ptr, op_data->target.slot, params_info.params,
                (archi_context_callback_t){0}, ARCHI_ERROR_PARAMETER);

        // Finalize the parameter list
        archi_context_registry_final_params(params_info);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__set)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__set_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__set_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("target.key", op_data->target.key);
        archi_print_slot("target.slot", op_data->target.slot);
        archi_print_value("value", op_data->value);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->target.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target context key is NULL");
            return;
        }

        // Obtain the target context
        archi_rcpointer_t target_context;
        {
            archi_error_t error = {0};
            target_context = archi_context_registry_get(context, op_data->target.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Set the target context slot
        archi_context_set(target_context.ptr, op_data->target.slot,
                archi_pointer_with_refcount(op_data->value, argument.ref_count),
                ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__assign)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__assign_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__assign_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("target.key", op_data->target.key);
        archi_print_slot("target.slot", op_data->target.slot);
        archi_print_key("source.key", op_data->source.key);
        archi_print_slot("source.slot", op_data->source.slot);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->target.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target context key is NULL");
            return;
        }
        else if (op_data->source.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context key is NULL");
            return;
        }

        // Obtain the target context
        archi_rcpointer_t target_context;
        {
            archi_error_t error = {0};
            target_context = archi_context_registry_get(context, op_data->target.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Obtain the source context
        archi_rcpointer_t source_context;
        {
            archi_error_t error = {0};
            source_context = archi_context_registry_get(context, op_data->source.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Assign the context slot
        archi_context_set_from_get(target_context.ptr, op_data->target.slot,
                source_context.ptr, op_data->source.slot, false,
                ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__weak_assign)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__assign_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__assign_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("target.key", op_data->target.key);
        archi_print_slot("target.slot", op_data->target.slot);
        archi_print_key("source.key", op_data->source.key);
        archi_print_slot("source.slot", op_data->source.slot);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->target.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target context key is NULL");
            return;
        }
        else if (op_data->source.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context key is NULL");
            return;
        }

        // Obtain the target context
        archi_rcpointer_t target_context;
        {
            archi_error_t error = {0};
            target_context = archi_context_registry_get(context, op_data->target.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Obtain the source context
        archi_rcpointer_t source_context;
        {
            archi_error_t error = {0};
            source_context = archi_context_registry_get(context, op_data->source.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Assign the context slot
        archi_context_set_from_get(target_context.ptr, op_data->target.slot,
                source_context.ptr, op_data->source.slot, true,
                ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__assign_call)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__assign_call_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__assign_call_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("target.key", op_data->target.key);
        archi_print_slot("target.slot", op_data->target.slot);
        archi_print_key("source.key", op_data->source.key);
        archi_print_slot("source.slot", op_data->source.slot);
        archi_print_key("call_params.list_key", op_data->call_params.list_key);
        archi_print_params("call_params.list", op_data->call_params.list);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->target.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target context key is NULL");
            return;
        }
        else if (op_data->source.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context key is NULL");
            return;
        }

        // Obtain the target context
        archi_rcpointer_t target_context;
        {
            archi_error_t error = {0};
            target_context = archi_context_registry_get(context, op_data->target.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Obtain the source context
        archi_rcpointer_t source_context;
        {
            archi_error_t error = {0};
            source_context = archi_context_registry_get(context, op_data->source.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Prepare the parameter list
        struct archi_context_params_info params_info;
        {
            archi_error_t error = {0};
            params_info = archi_context_registry_init_params(context,
                    op_data->call_params.list_key, op_data->call_params.list,
                    argument.ref_count, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Invoke the context action and assign the context slot
        archi_context_set_from_call(target_context.ptr, op_data->target.slot,
                source_context.ptr, op_data->source.slot, params_info.params, false,
                ARCHI_ERROR_PARAMETER);

        // Finalize the parameter list
        archi_context_registry_final_params(params_info);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__weak_assign_call)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__assign_call_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__assign_call_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("target.key", op_data->target.key);
        archi_print_slot("target.slot", op_data->target.slot);
        archi_print_key("source.key", op_data->source.key);
        archi_print_slot("source.slot", op_data->source.slot);
        archi_print_key("call_params.list_key", op_data->call_params.list_key);
        archi_print_params("call_params.list", op_data->call_params.list);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->target.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "target context key is NULL");
            return;
        }
        else if (op_data->source.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "source context key is NULL");
            return;
        }

        // Obtain the target context
        archi_rcpointer_t target_context;
        {
            archi_error_t error = {0};
            target_context = archi_context_registry_get(context, op_data->target.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Obtain the source context
        archi_rcpointer_t source_context;
        {
            archi_error_t error = {0};
            source_context = archi_context_registry_get(context, op_data->source.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Prepare the parameter list
        struct archi_context_params_info params_info;
        {
            archi_error_t error = {0};
            params_info = archi_context_registry_init_params(context,
                    op_data->call_params.list_key, op_data->call_params.list,
                    argument.ref_count, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Invoke the context action and assign the context slot
        archi_context_set_from_call(target_context.ptr, op_data->target.slot,
                source_context.ptr, op_data->source.slot, params_info.params, true,
                ARCHI_ERROR_PARAMETER);

        // Finalize the parameter list
        archi_context_registry_final_params(params_info);
    }
}

/*****************************************************************************/

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_parameters)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__create_parameters_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__create_parameters_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("key", op_data->key);
        archi_print_key("init_params.list_key", op_data->init_params.list_key);
        archi_print_params("init_params.list", op_data->init_params.list);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is NULL");
            return;
        }
        else if (op_data->key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is empty");
            return;
        }

        // Prepare the context interface
        archi_rcpointer_t interface_ptr = {
            .cptr = &archi_context_interface__parameters,
            .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_interface_t),
        };

        // Initialize the context and add it to the registry
        archi_context_registry_add(context, op_data->key, interface_ptr,
                op_data->init_params.list_key, op_data->init_params.list,
                argument.ref_count, NULL, ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_ptr_to_value)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__create_ptr_to_value_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__create_ptr_to_value_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("key", op_data->key);
        archi_print_value("value", op_data->value);
        archi_print_pointer_type("type", op_data->type);
        archi_print_params("init_params_list", op_data->init_params_list);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is NULL");
            return;
        }
        else if (op_data->key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is empty");
            return;
        }

        // Prepare the context interface
        archi_rcpointer_t interface_ptr = {
            .cptr = archi_context_interface_for_pointer_type(op_data->type),
            .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_interface_t),
        };

        if (interface_ptr.cptr == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unknown pointer interface type (%i)",
                    op_data->type);
            return;
        }

        // Initialize the context and add it to the registry
        archi_kvlist_rc_t params[] = {
            {
                .key = "pointee",
                .value = archi_pointer_with_refcount(op_data->value, argument.ref_count),
            },
        };

        archi_context_registry_add(context, op_data->key, interface_ptr,
                NULL, op_data->init_params_list,
                argument.ref_count, params, ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_ptr_to_context)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__create_ptr_to_context_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__create_ptr_to_context_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("key", op_data->key);
        archi_print_key("pointee.key", op_data->pointee.key);
        archi_print_slot("pointee.slot", op_data->pointee.slot);
        archi_print_pointer_type("type", op_data->type);
        archi_print_params("init_params_list", op_data->init_params_list);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is NULL");
            return;
        }
        else if (op_data->key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is empty");
            return;
        }
        else if (op_data->pointee.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointee context key is NULL");
            return;
        }
        else if (op_data->pointee.key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointee context key is empty");
            return;
        }

        // Prepare the context interface
        archi_rcpointer_t interface_ptr = {
            .cptr = archi_context_interface_for_pointer_type(op_data->type),
            .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_interface_t),
        };

        if (interface_ptr.cptr == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unknown pointer interface type (%i)",
                    op_data->type);
            return;
        }

        // Obtain the pointee
        archi_rcpointer_t pointee;
        {
            archi_error_t error = {0};
            archi_rcpointer_t pointee_context = archi_context_registry_get(context, op_data->pointee.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;

            // Get interface from the origin context slot
            ARCHI_ERROR_RESET_VAR(&error);
            archi_context_get(pointee_context.ptr, op_data->pointee.slot,
                    (archi_context_callback_t){.function = archi_context_callback__getter,
                    .data = &pointee}, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Initialize the context and add it to the registry
        archi_kvlist_rc_t params[] = {
            {
                .key = "pointee",
                .value = pointee,
            },
        };

        archi_context_registry_add(context, op_data->key, interface_ptr,
                NULL, op_data->init_params_list,
                argument.ref_count, params, ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_weak_ptr_to_context)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__create_ptr_to_context_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__create_ptr_to_context_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("key", op_data->key);
        archi_print_key("pointee.key", op_data->pointee.key);
        archi_print_slot("pointee.slot", op_data->pointee.slot);
        archi_print_pointer_type("type", op_data->type);
        archi_print_params("init_params_list", op_data->init_params_list);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is NULL");
            return;
        }
        else if (op_data->key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is empty");
            return;
        }
        else if (op_data->pointee.key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointee context key is NULL");
            return;
        }
        else if (op_data->pointee.key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "pointee context key is empty");
            return;
        }

        // Prepare the context interface
        archi_rcpointer_t interface_ptr = {
            .cptr = archi_context_interface_for_pointer_type(op_data->type),
            .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_interface_t),
        };

        if (interface_ptr.cptr == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "unknown pointer interface type (%i)",
                    op_data->type);
            return;
        }

        // Obtain the pointee
        archi_rcpointer_t pointee;
        {
            archi_error_t error = {0};
            archi_rcpointer_t pointee_context = archi_context_registry_get(context, op_data->pointee.key, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;

            // Get interface from the origin context slot
            ARCHI_ERROR_RESET_VAR(&error);
            archi_context_get(pointee_context.ptr, op_data->pointee.slot,
                    (archi_context_callback_t){.function = archi_context_callback__getter,
                    .data = &pointee}, &error);
            ARCHI_ERROR_ASSIGN(error);

            if (error.code != 0)
                return;
        }

        // Reset the reference counter
        pointee.ref_count = NULL;

        // Initialize the context and add it to the registry
        archi_kvlist_rc_t params[] = {
            {
                .key = "pointee",
                .value = pointee,
            },
        };

        archi_context_registry_add(context, op_data->key, interface_ptr,
                NULL, op_data->init_params_list,
                argument.ref_count, params, ARCHI_ERROR_PARAMETER);
    }
}

ARCHI_CONTEXT_OPERATION_FUNC(archi_context_registry_op__create_dptr_array)
{
    if (!archi_pointer_attr_compatible(argument.attr,
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_registry_op_data__create_dptr_array_t)))
    {
        ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "context operation argument has incorrect type");
        return;
    }

    const archi_context_registry_op_data__create_dptr_array_t *op_data = argument.ptr;

    if (context == NULL)
    {
        // Print operation data
        archi_print_key("key", op_data->key);
        archi_print("length = %zu\n", op_data->length);

        ARCHI_ERROR_RESET();
    }
    else
    {
        // Validate operation data
        if (op_data->key == NULL)
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is NULL");
            return;
        }
        else if (op_data->key[0] == '\0')
        {
            ARCHI_ERROR_SET(ARCHI__ECONSTRAINT, "created context key is empty");
            return;
        }

        // Prepare the context interface
        archi_rcpointer_t interface_ptr = {
            .cptr = &archi_context_interface__dptr_array,
            .attr = ARCHI_POINTER_TYPE__DATA_READONLY |
                ARCHI_POINTER_ATTR__DATA_TYPE(1, archi_context_interface_t),
        };

        // Initialize the context and add it to the registry
        size_t length = op_data->length;

        archi_kvlist_rc_t params[] = {
            {
                .key = "length",
                .value = {
                    .ptr = &length,
                    .attr = ARCHI_POINTER_TYPE__DATA_ON_STACK |
                        ARCHI_POINTER_ATTR__DATA_TYPE(1, size_t),
                },
            },
        };

        archi_context_registry_add(context, op_data->key, interface_ptr,
                NULL, NULL,
                argument.ref_count, params, ARCHI_ERROR_PARAMETER);
    }
}

