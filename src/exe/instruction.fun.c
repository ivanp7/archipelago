/*****************************************************************************
 * Copyright (C) 2023-2025 by Ivan Podmazov                                  *
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
 * @brief Context registry instructions.
 */

#include "archi/exe/instruction.fun.h"
#include "archi/exe/instruction.typ.h"
#include "archi/ctx/interface.fun.h"
#include "archi/ctx/interface/parameters.var.h"
#include "archi/ctx/interface/pointer.var.h"
#include "archi/log/print.fun.h"
#include "archi/log/print.def.h"
#include "archi/log/color.def.h"
#include "archi/log/context.fun.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy()
#include <stdbool.h>

#define MAX_BYTES       32

#define MAX_CHARS       (MAX_BYTES / sizeof(char))
#define MAX_SHORTS      (MAX_BYTES / sizeof(short))
#define MAX_INTS        (MAX_BYTES / sizeof(int))
#define MAX_LONGS       (MAX_BYTES / sizeof(long))
#define MAX_LONGLONGS   (MAX_BYTES / sizeof(long long))
#define MAX_FLOATS      (MAX_BYTES / sizeof(float))
#define MAX_DOUBLES     (MAX_BYTES / sizeof(double))
#define MAX_LONGDOUBLES (MAX_BYTES / sizeof(long double))

size_t
archi_exe_registry_instr_sizeof(
        const archi_exe_registry_instr_base_t *instruction)
{
    if (instruction == NULL)
        return 0;

    switch (instruction->type)
    {
        case ARCHI_EXE_REGISTRY_INSTR_INIT_FROM_CONTEXT:
            return sizeof(archi_exe_registry_instr_init_from_context_t);

        case ARCHI_EXE_REGISTRY_INSTR_INIT_FROM_SLOT:
            return sizeof(archi_exe_registry_instr_init_from_slot_t);

        case ARCHI_EXE_REGISTRY_INSTR_FINAL:
            return sizeof(archi_exe_registry_instr_final_t);

        case ARCHI_EXE_REGISTRY_INSTR_SET_TO_VALUE:
            return sizeof(archi_exe_registry_instr_set_to_value_t);

        case ARCHI_EXE_REGISTRY_INSTR_SET_TO_CONTEXT:
            return sizeof(archi_exe_registry_instr_set_to_context_t);

        case ARCHI_EXE_REGISTRY_INSTR_SET_TO_SLOT:
            return sizeof(archi_exe_registry_instr_set_to_slot_t);

        case ARCHI_EXE_REGISTRY_INSTR_ACT:
            return sizeof(archi_exe_registry_instr_act_t);

        default:
            return sizeof(archi_exe_registry_instr_base_t);
    }
}

static
void
archi_print_value(
        const char *indent,
        archi_pointer_t value)
{
    // Print attributes and flags
    {
        if ((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0)
        {
            if (value.flags & ARCHI_POINTER_FLAG_WRITABLE)
                archi_print("WRITABLE_DATA");
            else
                archi_print("READ_ONLY_DATA");
        }

        else
            archi_print("FUNCTION");

        if ((value.flags & ARCHI_POINTER_USER_FLAGS_MASK) != 0)
            archi_print(" | 0x%X", value.flags & ARCHI_POINTER_USER_FLAGS_MASK);

        if (value.ref_count != NULL)
            archi_print(" (ref_count)\n");

        {
            archi_print(" (");

            archi_print("num = %zu", value.element.num_of);

            if (value.element.size != 0)
                archi_print(", size = %zu", value.element.size);

            if (value.element.alignment != 0)
                archi_print(", align = %zu", value.element.alignment);

            archi_print(")");
        }

        archi_print("\n");
    }

    // Print memory contents
    if (((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0) && (value.element.size != 0))
    {
        size_t alignment = (value.element.alignment != 0) ? value.element.alignment : 1;
        size_t size_padded = ARCHI_SIZE_PADDED(value.element.size, alignment);

        {
            archi_print("%s  bytes:", indent);

            for (size_t i = 0; i < MAX_BYTES; i++)
            {
                if (i >= value.element.num_of * value.element.size)
                    break;

                unsigned char val;
                memcpy(&val, (char*)value.ptr + i, sizeof(char));

                archi_print(" %02x", (int)val);
            }

            archi_print("\n");
        }

#define archi_print_ELEMENTS(max_elements, type, name, format) do {   \
            archi_print("%s  " name ":", indent);                 \
            for (size_t i = 0; i < (max_elements); i++)     \
            {                                               \
                if (i >= value.element.num_of)              \
                    break;                                  \
                                                            \
                type val;                                   \
                memcpy(&val, (char*)value.ptr +             \
                        i * size_padded, sizeof(type));     \
                                                            \
                archi_print(" " format, val);                     \
            }                                               \
            archi_print("\n");                                    \
        } while (0)

        if (value.element.size == sizeof(char))
        {
            archi_print("%s  string: ", indent);
            for (size_t i = 0; i < MAX_CHARS; i++)
            {
                if (i >= value.element.num_of)
                    break;

                unsigned char val;
                memcpy(&val, (char*)value.ptr + i * size_padded, sizeof(char));

                if (val >= 32)
                    archi_print("%c", (int)val);
                else
                    archi_print(".");
            }
            archi_print("\n");

            archi_print_ELEMENTS(MAX_CHARS, unsigned char, "uchar", "%hhu");
            archi_print_ELEMENTS(MAX_CHARS, signed char, "schar", "%hhi");
        }
        else if (value.element.size == sizeof(short))
        {
            archi_print_ELEMENTS(MAX_SHORTS, unsigned short, "ushort", "%hu");
            archi_print_ELEMENTS(MAX_SHORTS, signed char, "sshort", "%hi");
        }
        else if (value.element.size == sizeof(int))
        {
            archi_print_ELEMENTS(MAX_INTS, unsigned int, "uint", "%u");
            archi_print_ELEMENTS(MAX_INTS, signed int, "sint", "%i");
        }
        else if (value.element.size == sizeof(long))
        {
            archi_print_ELEMENTS(MAX_LONGS, unsigned long, "ulong", "%lu");
            archi_print_ELEMENTS(MAX_LONGS, signed long, "slong", "%li");
        }
        else if (value.element.size == sizeof(long long))
        {
            archi_print_ELEMENTS(MAX_LONGLONGS, unsigned long long, "ulonglong", "%llu");
            archi_print_ELEMENTS(MAX_LONGLONGS, signed long long, "slonglong", "%lli");
        }

        if (value.element.size == sizeof(float))
        {
            archi_print_ELEMENTS(MAX_FLOATS, float, "float", "%e");
        }
        else if (value.element.size == sizeof(double))
        {
            archi_print_ELEMENTS(MAX_DOUBLES, double, "double", "%e");
        }
        else if (value.element.size == sizeof(long double))
        {
            archi_print_ELEMENTS(MAX_LONGDOUBLES, long double, "longdouble", "%Le");
        }
    }
}

static
void
archi_exe_registry_instr_params_free(
        archi_parameter_list_t *params,
        const archi_parameter_list_t *dparams)
{
    archi_parameter_list_t *node = params;

    while (node != dparams)
    {
        archi_parameter_list_t *next = node->next;
        free(node);
        node = next;
    }
}

static
archi_parameter_list_t*
archi_exe_registry_instr_params_alloc(
        archi_parameter_list_t *dparams,
        const archi_parameter_list_t *sparams,
        archi_reference_count_t ref_count)
{
    archi_parameter_list_t *params = dparams;

    for (const archi_parameter_list_t *param = sparams; param != NULL; param = param->next)
    {
        archi_parameter_list_t *node = malloc(sizeof(*node));
        if (node == NULL)
        {
            archi_exe_registry_instr_params_free(params, dparams);
            return NULL;
        }

        *node = (archi_parameter_list_t){
            .next = params,
            .name = param->name,
            .value = param->value,
        };
        node->value.ref_count = ref_count;

        params = node;
    }

    return params;
}

static
archi_status_t
archi_exe_registry_instr_execute_init_from_context(
        archi_context_t registry,
        const archi_exe_registry_instr_init_from_context_t *instr_init_from_context,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    // Print the instruction details
    if (logging)
    {
        {
            archi_print(ARCHI_LOG_INDENT "    key = ");

            if (instr_init_from_context->key != NULL)
                archi_print("\"%s\"\n", instr_init_from_context->key);
            else
                archi_print("NULL\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    interface_source_key = ");

            if (instr_init_from_context->interface_source_key == NULL)
                archi_print("<parameter list>\n");
            else if (instr_init_from_context->interface_source_key[0] == '\0')
                archi_print("<copied pointer>\n");
            else
                archi_print("\"%s\"\n", instr_init_from_context->interface_source_key);
        }

        if (instr_init_from_context->dparams_key != NULL)
            archi_print(ARCHI_LOG_INDENT "    dparams_key = \"%s\"\n", instr_init_from_context->dparams_key);

        if (instr_init_from_context->sparams != NULL)
        {
            archi_print(ARCHI_LOG_INDENT "    sparams:\n");

            for (const archi_parameter_list_t *params = instr_init_from_context->sparams;
                    params != NULL; params = params->next)
            {
                archi_print(ARCHI_LOG_INDENT "      \"%s\": ", params->name);
                archi_print_value(ARCHI_LOG_INDENT "        ", params->value);
            }
        }
    }

    if (dry_run)
        return 0;

    if ((instr_init_from_context->key == NULL) ||
            (instr_init_from_context->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Check early if the context key exists already
    archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_init_from_context->key}, &code);

    if (code != 1)
    {
        if (code < 0)
            return code;
        else if (code == 0)
            return 2; // the context key exists already
        else
            return ARCHI_STATUS_EFAILURE;
    }

    // Obtain the context interface
    archi_pointer_t interface_value;

    if (instr_init_from_context->interface_source_key == NULL) // parameter list
        interface_value = (archi_pointer_t){
            .ptr = (void*)&archi_context_parameters_interface,
            .element = {
                .num_of = 1,
            },
        };
    else if (instr_init_from_context->interface_source_key[0] == '\0') // pointer copy
        interface_value = (archi_pointer_t){
            .ptr = (void*)&archi_context_pointer_interface,
            .element = {
                .num_of = 1,
            },
        };
    else
    {
        // Obtain the source context from the registry
        archi_pointer_t src_context_value = archi_context_get_slot(registry,
                (archi_context_op_designator_t){.name = instr_init_from_context->interface_source_key}, &code);

        if (code != 0)
        {
            if (code < 0)
                return code;
            else if (code == 1)
                return 1; // the context is not found
            else
                return ARCHI_STATUS_EFAILURE;
        }
        else if ((src_context_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                (src_context_value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        // Get context interface from the source context
        interface_value = archi_context_interface(src_context_value.ptr);
    }

    // Prepare the context initialization parameter list
    archi_parameter_list_t *dparams = NULL;
    if (instr_init_from_context->dparams_key != NULL)
    {
        // Get dynamic parameter list
        archi_pointer_t dparams_value = archi_context_get_slot(registry,
                (archi_context_op_designator_t){.name = instr_init_from_context->dparams_key}, &code);

        if (code != 0)
        {
            if (code < 0)
                return code;
            else if (code == 1)
                return 1; // the parameter list is not found
            else
                return ARCHI_STATUS_EFAILURE;
        }

        if (dparams_value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        dparams = dparams_value.ptr;
    }

    archi_parameter_list_t *params = archi_exe_registry_instr_params_alloc(
            dparams, instr_init_from_context->sparams, ref_count);
    if ((params == NULL) && ((dparams != NULL) || (instr_init_from_context->sparams != NULL)))
        return ARCHI_STATUS_ENOMEMORY;

    // Initialize the context
    archi_context_t context = archi_context_initialize(interface_value, params, &code);

    archi_exe_registry_instr_params_free(params, dparams);

    if (context == NULL)
        return ARCHI_STATUS_TO_ERROR(code);

    archi_pointer_t context_value = {
        .ptr = context,
        .ref_count = archi_context_data(context).ref_count, // the reference count is 1 at this point
        .element = {
            .num_of = 1,
        },
    };

    // Insert the context to the registry, which also increments the reference count
    code = archi_context_set_slot(registry,
            (archi_context_op_designator_t){.name = instr_init_from_context->key}, context_value);

    if (code != 0)
    {
        archi_context_finalize(context);
        return ARCHI_STATUS_TO_ERROR(code);
    }

    // Decrement the reference count back to 1, making
    archi_reference_count_decrement(context_value.ref_count);

    return 0;
}

static
archi_status_t
archi_exe_registry_instr_execute_init_from_slot(
        archi_context_t registry,
        const archi_exe_registry_instr_init_from_slot_t *instr_init_from_slot,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    // Print the instruction details
    if (logging)
    {
        {
            archi_print(ARCHI_LOG_INDENT "    key = ");

            if (instr_init_from_slot->key != NULL)
                archi_print("\"%s\"\n", instr_init_from_slot->key);
            else
                archi_print("NULL\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    interface_source_key = ");

            if (instr_init_from_slot->interface_source_key != NULL)
                archi_print("\"%s\"\n", instr_init_from_slot->interface_source_key);
            else
                archi_print("NULL\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    interface_source_slot.name = ");

            if (instr_init_from_slot->interface_source_slot.name != NULL)
                archi_print("\"%s\"\n", instr_init_from_slot->interface_source_slot.name);
            else
                archi_print("NULL\n");
        }

        if (instr_init_from_slot->interface_source_slot.num_indices > 0)
        {
            archi_print(ARCHI_LOG_INDENT "    interface_source_slot.indices[%zu] =",
                    instr_init_from_slot->interface_source_slot.num_indices);

            for (size_t i = 0; i < instr_init_from_slot->interface_source_slot.num_indices; i++)
                archi_print(" %tu", instr_init_from_slot->interface_source_slot.index[i]);

            archi_print("\n");
        }

        if (instr_init_from_slot->dparams_key != NULL)
            archi_print(ARCHI_LOG_INDENT "    dparams_key = \"%s\"\n", instr_init_from_slot->dparams_key);

        if (instr_init_from_slot->sparams != NULL)
        {
            archi_print(ARCHI_LOG_INDENT "    sparams:\n");

            for (const archi_parameter_list_t *params = instr_init_from_slot->sparams;
                    params != NULL; params = params->next)
            {
                archi_print(ARCHI_LOG_INDENT "      \"%s\": ", params->name);
                archi_print_value(ARCHI_LOG_INDENT "        ", params->value);
            }
        }
    }

    if (dry_run)
        return 0;

    if ((instr_init_from_slot->key == NULL) ||
            (instr_init_from_slot->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Check early if the context key exists already
    archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_init_from_slot->key}, &code);

    if (code != 1)
    {
        if (code < 0)
            return code;
        else if (code == 0)
            return 2; // the context key exists already
        else
            return ARCHI_STATUS_EFAILURE;
    }

    // Obtain the context interface
    archi_pointer_t interface_value;
    {
        // Obtain the source context from the registry
        archi_pointer_t src_context_value = archi_context_get_slot(registry,
                (archi_context_op_designator_t){.name = instr_init_from_slot->interface_source_key}, &code);

        if (code != 0)
        {
            if (code < 0)
                return code;
            else if (code == 1)
                return 1; // the context is not found
            else
                return ARCHI_STATUS_EFAILURE;
        }
        else if ((src_context_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                (src_context_value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;

        // Get context interface from the source context slot
        interface_value = archi_context_get_slot(src_context_value.ptr,
                instr_init_from_slot->interface_source_slot, &code);

        if (code != 0)
            return ARCHI_STATUS_TO_ERROR(code);
    }

    // Prepare the context initialization parameter list
    archi_parameter_list_t *dparams = NULL;
    if (instr_init_from_slot->dparams_key != NULL)
    {
        // Get dynamic parameter list
        archi_pointer_t dparams_value = archi_context_get_slot(registry,
                (archi_context_op_designator_t){.name = instr_init_from_slot->dparams_key}, &code);

        if (code != 0)
        {
            if (code < 0)
                return code;
            else if (code == 1)
                return 1; // the parameter list is not found
            else
                return ARCHI_STATUS_EFAILURE;
        }

        if (dparams_value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        dparams = dparams_value.ptr;
    }

    archi_parameter_list_t *params = archi_exe_registry_instr_params_alloc(
            dparams, instr_init_from_slot->sparams, ref_count);
    if ((params == NULL) && ((dparams != NULL) || (instr_init_from_slot->sparams != NULL)))
        return ARCHI_STATUS_ENOMEMORY;

    // Initialize the context
    archi_context_t context = archi_context_initialize(interface_value, params, &code);

    archi_exe_registry_instr_params_free(params, dparams);

    if (context == NULL)
        return ARCHI_STATUS_TO_ERROR(code);

    archi_pointer_t context_value = {
        .ptr = context,
        .ref_count = archi_context_data(context).ref_count, // the reference count is 1 at this point
        .element = {
            .num_of = 1,
        },
    };

    // Insert the context to the registry, which also increments the reference count
    code = archi_context_set_slot(registry,
            (archi_context_op_designator_t){.name = instr_init_from_slot->key}, context_value);

    if (code != 0)
    {
        archi_context_finalize(context);
        return ARCHI_STATUS_TO_ERROR(code);
    }

    // Decrement the reference count back to 1, making
    archi_reference_count_decrement(context_value.ref_count);

    return 0;
}

static
archi_status_t
archi_exe_registry_instr_execute_final(
        archi_context_t registry,
        const archi_exe_registry_instr_final_t *instr_final,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    (void) ref_count;

    // Print the instruction details
    if (logging)
    {
        {
            archi_print(ARCHI_LOG_INDENT "    key = ");

            if (instr_final->key != NULL)
                archi_print("\"%s\"\n", instr_final->key);
            else
                archi_print("NULL\n");
        }
    }

    if (dry_run)
        return 0;

    if ((instr_final->key == NULL) || (instr_final->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    // Remove the context from the registry, which also decrements the reference count
    archi_status_t code = archi_context_set_slot(registry,
            (archi_context_op_designator_t){.name = instr_final->key}, (archi_pointer_t){0});

    if (code != 0)
    {
        if (code < 0)
            return code;
        else if (code == 1)
            return 1; // the context is not found
        else
            return ARCHI_STATUS_EFAILURE;
    }

    return 0;
}

static
archi_status_t
archi_exe_registry_instr_execute_set_to_value(
        archi_context_t registry,
        const archi_exe_registry_instr_set_to_value_t *instr_set_to_value,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    // Print the instruction details
    if (logging)
    {
        {
            archi_print(ARCHI_LOG_INDENT "    key = ");

            if (instr_set_to_value->key != NULL)
                archi_print("\"%s\"\n", instr_set_to_value->key);
            else
                archi_print("NULL\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    slot.name = ");

            if (instr_set_to_value->slot.name != NULL)
                archi_print("\"%s\"\n", instr_set_to_value->slot.name);
            else
                archi_print("NULL\n");
        }

        if (instr_set_to_value->slot.num_indices > 0)
        {
            archi_print(ARCHI_LOG_INDENT "    slot.indices[%zu] =", instr_set_to_value->slot.num_indices);

            for (size_t i = 0; i < instr_set_to_value->slot.num_indices; i++)
                archi_print(" %tu", instr_set_to_value->slot.index[i]);

            archi_print("\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    value = ");
            archi_print_value(ARCHI_LOG_INDENT "      ", instr_set_to_value->value);
        }
    }

    if (dry_run)
        return 0;

    if ((instr_set_to_value->key == NULL) || (instr_set_to_value->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_set_to_value->key}, &code);

    if (code != 0)
    {
        if (code < 0)
            return code;
        else if (code == 1)
            return 1; // the context is not found
        else
            return ARCHI_STATUS_EFAILURE;
    }
    else if ((context_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
            (context_value.ptr == NULL))
        return ARCHI_STATUS_EVALUE;

    // Set the context value
    archi_pointer_t value = instr_set_to_value->value;
    value.ref_count = ref_count;

    code = archi_context_set_slot(context_value.ptr, instr_set_to_value->slot, value);

    if (code != 0)
        return ARCHI_STATUS_TO_ERROR(code);

    return 0;
}

static
archi_status_t
archi_exe_registry_instr_execute_set_to_context(
        archi_context_t registry,
        const archi_exe_registry_instr_set_to_context_t *instr_set_to_context,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    (void) ref_count;

    // Print the instruction details
    if (logging)
    {
        {
            archi_print(ARCHI_LOG_INDENT "    key = ");

            if (instr_set_to_context->key != NULL)
                archi_print("\"%s\"\n", instr_set_to_context->key);
            else
                archi_print("NULL\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    slot.name = ");

            if (instr_set_to_context->slot.name != NULL)
                archi_print("\"%s\"\n", instr_set_to_context->slot.name);
            else
                archi_print("NULL\n");
        }

        if (instr_set_to_context->slot.num_indices > 0)
        {
            archi_print(ARCHI_LOG_INDENT "    slot.indices[%zu] =", instr_set_to_context->slot.num_indices);

            for (size_t i = 0; i < instr_set_to_context->slot.num_indices; i++)
                archi_print(" %tu", instr_set_to_context->slot.index[i]);

            archi_print("\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    source_key = ");

            if (instr_set_to_context->source_key != NULL)
                archi_print("\"%s\"\n", instr_set_to_context->source_key);
            else
                archi_print("NULL\n");
        }
    }

    if (dry_run)
        return 0;

    if ((instr_set_to_context->key == NULL) || (instr_set_to_context->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;
    else if ((instr_set_to_context->source_key == NULL) || (instr_set_to_context->source_key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_set_to_context->key}, &code);

    if (code != 0)
    {
        if (code < 0)
            return code;
        else if (code == 1)
            return 1; // the context is not found
        else
            return ARCHI_STATUS_EFAILURE;
    }
    else if ((context_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
            (context_value.ptr == NULL))
        return ARCHI_STATUS_EVALUE;

    // Obtain the source context from the registry
    archi_pointer_t src_context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_set_to_context->source_key}, &code);

    if (code != 0)
    {
        if (code < 0)
            return code;
        else if (code == 1)
            return 1; // the context is not found
        else
            return ARCHI_STATUS_EFAILURE;
    }
    else if ((src_context_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
            (src_context_value.ptr == NULL))
        return ARCHI_STATUS_EVALUE;

    // Set the context value
    code = archi_context_set_slot(context_value.ptr, instr_set_to_context->slot,
            archi_context_data(src_context_value.ptr));

    if (code != 0)
        return ARCHI_STATUS_TO_ERROR(code);

    return 0;
}

static
archi_status_t
archi_exe_registry_instr_execute_set_to_slot(
        archi_context_t registry,
        const archi_exe_registry_instr_set_to_slot_t *instr_set_to_slot,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    (void) ref_count;

    // Print the instruction details
    if (logging)
    {
        {
            archi_print(ARCHI_LOG_INDENT "    key = ");

            if (instr_set_to_slot->key != NULL)
                archi_print("\"%s\"\n", instr_set_to_slot->key);
            else
                archi_print("NULL\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    slot.name = ");

            if (instr_set_to_slot->slot.name != NULL)
                archi_print("\"%s\"\n", instr_set_to_slot->slot.name);
            else
                archi_print("NULL\n");
        }

        if (instr_set_to_slot->slot.num_indices > 0)
        {
            archi_print(ARCHI_LOG_INDENT "    slot.indices[%zu] =", instr_set_to_slot->slot.num_indices);

            for (size_t i = 0; i < instr_set_to_slot->slot.num_indices; i++)
                archi_print(" %tu", instr_set_to_slot->slot.index[i]);

            archi_print("\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    source_key = ");

            if (instr_set_to_slot->source_key != NULL)
                archi_print("\"%s\"\n", instr_set_to_slot->source_key);
            else
                archi_print("NULL\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    source_slot.name = ");

            if (instr_set_to_slot->source_slot.name != NULL)
                archi_print("\"%s\"\n", instr_set_to_slot->source_slot.name);
            else
                archi_print("NULL\n");
        }

        if (instr_set_to_slot->source_slot.num_indices > 0)
        {
            archi_print(ARCHI_LOG_INDENT "    source_slot.indices[%zu] =",
                    instr_set_to_slot->source_slot.num_indices);

            for (size_t i = 0; i < instr_set_to_slot->source_slot.num_indices; i++)
                archi_print(" %tu", instr_set_to_slot->source_slot.index[i]);

            archi_print("\n");
        }
    }

    if (dry_run)
        return 0;

    if ((instr_set_to_slot->key == NULL) || (instr_set_to_slot->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;
    else if ((instr_set_to_slot->source_key == NULL) || (instr_set_to_slot->source_key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_set_to_slot->key}, &code);

    if (code != 0)
    {
        if (code < 0)
            return code;
        else if (code == 1)
            return 1; // the context is not found
        else
            return ARCHI_STATUS_EFAILURE;
    }
    else if ((context_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
            (context_value.ptr == NULL))
        return ARCHI_STATUS_EVALUE;

    // Obtain the source context from the registry
    archi_pointer_t src_context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_set_to_slot->source_key}, &code);

    if (code != 0)
    {
        if (code < 0)
            return code;
        else if (code == 1)
            return 1; // the context is not found
        else
            return ARCHI_STATUS_EFAILURE;
    }
    else if ((src_context_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
            (src_context_value.ptr == NULL))
        return ARCHI_STATUS_EVALUE;

    // Set the context value
    code = archi_context_copy_slot(context_value.ptr, instr_set_to_slot->slot,
            src_context_value.ptr, instr_set_to_slot->source_slot);

    if (code != 0)
        return ARCHI_STATUS_TO_ERROR(code);

    return 0;
}

static
archi_status_t
archi_exe_registry_instr_execute_act(
        archi_context_t registry,
        const archi_exe_registry_instr_act_t *instr_act,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    // Print the instruction details
    if (logging)
    {
        {
            archi_print(ARCHI_LOG_INDENT "    key = ");

            if (instr_act->key != NULL)
                archi_print("\"%s\"\n", instr_act->key);
            else
                archi_print("NULL\n");
        }

        {
            archi_print(ARCHI_LOG_INDENT "    action.name = ");

            if (instr_act->action.name != NULL)
                archi_print("\"%s\"\n", instr_act->action.name);
            else
                archi_print("NULL\n");
        }

        if (instr_act->action.num_indices > 0)
        {
            archi_print(ARCHI_LOG_INDENT "    action.indices[%zu] =", instr_act->action.num_indices);

            for (size_t i = 0; i < instr_act->action.num_indices; i++)
                archi_print(" %tu", instr_act->action.index[i]);

            archi_print("\n");
        }

        if (instr_act->dparams_key != NULL)
            archi_print(ARCHI_LOG_INDENT "    dparams_key = \"%s\"\n", instr_act->dparams_key);

        if (instr_act->sparams != NULL)
        {
            archi_print(ARCHI_LOG_INDENT "    sparams:\n");

            for (const archi_parameter_list_t *params = instr_act->sparams;
                    params != NULL; params = params->next)
            {
                archi_print(ARCHI_LOG_INDENT "      \"%s\": ", params->name);
                archi_print_value(ARCHI_LOG_INDENT "        ", params->value);
            }
        }
    }

    if (dry_run)
        return 0;

    if ((instr_act->key == NULL) || (instr_act->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_act->key}, &code);

    if (code != 0)
    {
        if (code < 0)
            return code;
        else if (code == 1)
            return 1; // the context is not found
        else
            return ARCHI_STATUS_EFAILURE;
    }
    else if ((context_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
            (context_value.ptr == NULL))
        return ARCHI_STATUS_EVALUE;

    // Obtain the context action parameters
    archi_parameter_list_t *dparams = NULL;
    if (instr_act->dparams_key != NULL)
    {
        archi_pointer_t dparams_value = archi_context_get_slot(registry,
                (archi_context_op_designator_t){.name = instr_act->dparams_key}, &code);

        if (code != 0)
        {
            if (code < 0)
                return code;
            else if (code == 1)
                return 1; // the parameter list is not found
            else
                return ARCHI_STATUS_EFAILURE;
        }

        if (dparams_value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            return ARCHI_STATUS_EVALUE;

        dparams = dparams_value.ptr;
    }

    archi_parameter_list_t *params = archi_exe_registry_instr_params_alloc(dparams, instr_act->sparams, ref_count);
    if ((params == NULL) && ((dparams != NULL) || (instr_act->sparams != NULL)))
        return ARCHI_STATUS_ENOMEMORY;

    // Invoke the context action
    code = archi_context_act(context_value.ptr, instr_act->action, params);

    archi_exe_registry_instr_params_free(params, dparams);

    if (code != 0)
        return ARCHI_STATUS_TO_ERROR(code);

    return 0;
}

archi_status_t
archi_exe_registry_instr_execute(
        archi_context_t registry,
        const archi_exe_registry_instr_base_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run)
{
    if (registry == NULL)
        return ARCHI_STATUS_EMISUSE;

    bool logging = archi_print_lock(ARCHI_LOG_VERBOSITY_DEBUG);

    if (logging)
    {
        if (archi_log_colorful())
            archi_print(ARCHI_LOG_COLOR_DEBUG);
    }

    if ((instruction == NULL) || (instruction->type == ARCHI_EXE_REGISTRY_INSTR_NOOP))
    {
        if (logging)
        {
            archi_print(ARCHI_LOG_INDENT "instruction(NOOP)\n");

            archi_print(ARCHI_COLOR_RESET);
            archi_print_unlock();
        }

        return 0;
    }

    archi_status_t code;

    switch (instruction->type)
    {
        case ARCHI_EXE_REGISTRY_INSTR_INIT_FROM_CONTEXT:
            if (logging)
                archi_print(ARCHI_LOG_INDENT "instruction(INIT_FROM_CONTEXT)\n");

            code = archi_exe_registry_instr_execute_init_from_context(registry,
                    (const archi_exe_registry_instr_init_from_context_t*)instruction, ref_count,
                    dry_run, logging);
            break;

        case ARCHI_EXE_REGISTRY_INSTR_INIT_FROM_SLOT:
            if (logging)
                archi_print(ARCHI_LOG_INDENT "instruction(INIT_FROM_SLOT)\n");

            code = archi_exe_registry_instr_execute_init_from_slot(registry,
                    (const archi_exe_registry_instr_init_from_slot_t*)instruction, ref_count,
                    dry_run, logging);
            break;

        case ARCHI_EXE_REGISTRY_INSTR_FINAL:
            if (logging)
                archi_print(ARCHI_LOG_INDENT "instruction(FINAL)\n");

            code = archi_exe_registry_instr_execute_final(registry,
                    (const archi_exe_registry_instr_final_t*)instruction, ref_count,
                    dry_run, logging);
            break;

        case ARCHI_EXE_REGISTRY_INSTR_SET_TO_VALUE:
            if (logging)
                archi_print(ARCHI_LOG_INDENT "instruction(SET_TO_VALUE)\n");

            code = archi_exe_registry_instr_execute_set_to_value(registry,
                    (const archi_exe_registry_instr_set_to_value_t*)instruction, ref_count,
                    dry_run, logging);
            break;

        case ARCHI_EXE_REGISTRY_INSTR_SET_TO_CONTEXT:
            if (logging)
                archi_print(ARCHI_LOG_INDENT "instruction(SET_TO_CONTEXT)\n");

            code = archi_exe_registry_instr_execute_set_to_context(registry,
                    (const archi_exe_registry_instr_set_to_context_t*)instruction, ref_count,
                    dry_run, logging);
            break;

        case ARCHI_EXE_REGISTRY_INSTR_SET_TO_SLOT:
            if (logging)
                archi_print(ARCHI_LOG_INDENT "instruction(SET_TO_SLOT)\n");

            code = archi_exe_registry_instr_execute_set_to_slot(registry,
                    (const archi_exe_registry_instr_set_to_slot_t*)instruction, ref_count,
                    dry_run, logging);
            break;

        case ARCHI_EXE_REGISTRY_INSTR_ACT:
            if (logging)
                archi_print(ARCHI_LOG_INDENT "instruction(ACT)\n");

            code = archi_exe_registry_instr_execute_act(registry,
                    (const archi_exe_registry_instr_act_t*)instruction, ref_count,
                    dry_run, logging);
            break;

        default:
            if (logging)
                archi_print(ARCHI_LOG_INDENT "instruction(<unknown>)\n");

            code = !dry_run ? ARCHI_STATUS_EMISUSE : 0;
    }

    if (logging)
    {
        if (archi_log_colorful())
            archi_print(ARCHI_COLOR_RESET);

        archi_print_unlock();
    }

    return code;
}

