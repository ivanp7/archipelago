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
#include "archi/log/context.fun.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy()
#include <stdbool.h>

#define PRINT(...) do { \
    archi_print(ARCHI_LOG_VERBOSITY_DEBUG, __VA_ARGS__); \
} while (0)

#define MAX_ELEMENTS    8
#define MAX_BYTES       16

size_t
archi_exe_registry_instr_sizeof(
        const archi_exe_registry_instr_base_t *instruction)
{
    if (instruction == NULL)
        return 0;

    switch (instruction->type)
    {
        case ARCHI_EXE_REGISTRY_INSTR_INIT:
            return sizeof(archi_exe_registry_instr_init_t);

        case ARCHI_EXE_REGISTRY_INSTR_SET_VALUE:
            return sizeof(archi_exe_registry_instr_set_value_t);

        case ARCHI_EXE_REGISTRY_INSTR_SET_CONTEXT:
            return sizeof(archi_exe_registry_instr_set_context_t);

        case ARCHI_EXE_REGISTRY_INSTR_SET_SLOT:
            return sizeof(archi_exe_registry_instr_set_slot_t);

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
        if (value.flags & ARCHI_POINTER_FLAG_FUNCTION)
            PRINT("FUNCTION");
        else
        {
            if (value.flags & ARCHI_POINTER_FLAG_WRITABLE)
                PRINT("WRITABLE_DATA");
            else
                PRINT("READ_ONLY_DATA");
        }

        if ((value.flags & ARCHI_POINTER_USER_FLAGS_MASK) != 0)
            PRINT(" | 0x%X", value.flags & ARCHI_POINTER_USER_FLAGS_MASK);

        if (value.ref_count != NULL)
            PRINT("    (ref_count)\n");

        PRINT("\n");
    }

    // Print array layout
    {
        PRINT("%snum_of = %zu", indent, value.element.num_of);

        if (value.element.size != 0)
            PRINT(", size = %zu", value.element.size);

        if (value.element.alignment != 0)
            PRINT(", alignment = %zu", value.element.alignment);

        PRINT("\n");
    }

    // Print memory contents
    if (((value.flags & ARCHI_POINTER_FLAG_FUNCTION) == 0) && (value.element.size != 0))
    {
        for (size_t i = 0; i < MAX_ELEMENTS; i++)
        {
            if (i >= value.element.num_of)
                break;

            PRINT("%s  [%zu]:", indent, i);

            size_t alignment = (value.element.alignment != 0) ? value.element.alignment : 1;
            size_t size_padded = ARCHI_SIZE_PADDED(value.element.size, alignment);

            // Print as integer
            if (value.element.size == sizeof(char))
            {
                unsigned char uval;
                signed char sval;

                memcpy(&uval, (char*)value.ptr + i * size_padded, sizeof(char));
                memcpy(&sval, (char*)value.ptr + i * size_padded, sizeof(char));

                if (uval >= 32)
                    PRINT(" '%c' x(%hhx) u(%hhu) i(%hhi)", (int)uval, uval, uval, sval);
                else
                    PRINT(" <.> x(%hhx) u(%hhu) i(%hhi)", uval, uval, sval);
            }
            else if (value.element.size == sizeof(short))
            {
                unsigned short uval;
                signed short sval;

                memcpy(&uval, (char*)value.ptr + i * size_padded, sizeof(short));
                memcpy(&sval, (char*)value.ptr + i * size_padded, sizeof(short));

                PRINT(" x(%hx), u(%hu), i(%hi)", uval, uval, sval);
            }
            else if (value.element.size == sizeof(int))
            {
                unsigned int uval;
                signed int sval;

                memcpy(&uval, (char*)value.ptr + i * size_padded, sizeof(int));
                memcpy(&sval, (char*)value.ptr + i * size_padded, sizeof(int));

                PRINT(" x(%x), u(%u), i(%i)", uval, uval, sval);
            }
            else if (value.element.size == sizeof(long))
            {
                unsigned long uval;
                signed long sval;

                memcpy(&uval, (char*)value.ptr + i * size_padded, sizeof(long));
                memcpy(&sval, (char*)value.ptr + i * size_padded, sizeof(long));

                PRINT(" x(%lx), u(%lu), i(%li)", uval, uval, sval);
            }
            else if (value.element.size == sizeof(long long))
            {
                unsigned long long uval;
                signed long long sval;

                memcpy(&uval, (char*)value.ptr + i * size_padded, sizeof(long long));
                memcpy(&sval, (char*)value.ptr + i * size_padded, sizeof(long long));

                PRINT(" x(%llx), u(%llu), i(%lli)", uval, uval, sval);
            }

            // Print as floating-point number
            if (value.element.size == sizeof(float))
            {
                float val;

                memcpy(&val, (char*)value.ptr + i * size_padded, sizeof(float));

                PRINT(" f(%e)", (double)val);
            }
            else if (value.element.size == sizeof(double))
            {
                double val;

                memcpy(&val, (char*)value.ptr + i * size_padded, sizeof(double));

                PRINT(" f(%e)", val);
            }
            else if (value.element.size == sizeof(long double))
            {
                long double val;

                memcpy(&val, (char*)value.ptr + i * size_padded, sizeof(long double));

                PRINT(" f(%Le)", val);
            }

            PRINT("\n%s   ", indent);

            for (size_t j = 0; j < MAX_BYTES; j++)
            {
                if (j >= value.element.size)
                    break;

                unsigned char uval;
                memcpy(&uval, (char*)value.ptr + i * size_padded + j, sizeof(char));

                PRINT(" %x", (int)uval);
            }

            PRINT("\n");
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

    while (params != dparams)
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
archi_exe_registry_instr_execute_init(
        archi_context_t registry,
        const archi_exe_registry_instr_init_t *instr_init,
        archi_reference_count_t ref_count,
        bool dry_run)
{
    // Print the instruction details
    if (archi_log_verbosity() >= ARCHI_LOG_VERBOSITY_DEBUG)
    {
        {
            PRINT(ARCHI_LOG_INDENT "interface_key = ");

            if (instr_init->interface_key == NULL)
                PRINT("<parameter list>\n");
            else if (instr_init->interface_key[0] == '\0')
                PRINT("<copied pointer>\n");
            else
                PRINT("\"%s\"\n", instr_init->interface_key);
        }

        {
            PRINT(ARCHI_LOG_INDENT "dparams_key = ");

            if (instr_init->dparams_key != NULL)
                PRINT("\"%s\"\n", instr_init->dparams_key);
            else
                PRINT("NULL\n");
        }

        {
            PRINT(ARCHI_LOG_INDENT "sparams:\n");

            for (const archi_parameter_list_t *params = instr_init->sparams;
                    params != NULL; params = params->next)
            {
                PRINT(ARCHI_LOG_INDENT "  %s = ", params->name);
                archi_print_value(ARCHI_LOG_INDENT "    ", params->value);
            }
        }
    }

    archi_print_unlock(ARCHI_LOG_VERBOSITY_DEBUG);

    if (dry_run)
        return 0;

    if ((instr_init->base.key == NULL) || (instr_init->base.key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Check early if the context key exists already
    archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_init->base.key}, &code);

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

    if (instr_init->interface_key == NULL) // parameter list
        interface_value = (archi_pointer_t){
            .ptr = (void*)&archi_context_parameters_interface,
            .element = {
                .num_of = 1,
            },
        };
    else if (instr_init->interface_key[0] == '\0') // pointer copy
        interface_value = (archi_pointer_t){
            .ptr = (void*)&archi_context_pointer_interface,
            .element = {
                .num_of = 1,
            },
        };
    else
    {
        interface_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_init->interface_key}, &code);

        if (code != 0)
        {
            if (code < 0)
                return code;
            else if (code == 1)
                return 1; // the interface is not found
            else
                return ARCHI_STATUS_EFAILURE;
        }
        else if ((interface_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                (interface_value.ptr == NULL))
            return ARCHI_STATUS_EVALUE;
    }

    // Prepare the context initialization parameter list
    archi_parameter_list_t *dparams = NULL;
    if (instr_init->dparams_key != NULL)
    {
        // Get dynamic parameter list
        archi_pointer_t dparams_value = archi_context_get_slot(registry,
                (archi_context_op_designator_t){.name = instr_init->dparams_key}, &code);

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

    archi_parameter_list_t *params = archi_exe_registry_instr_params_alloc(dparams, instr_init->sparams, ref_count);
    if (params == NULL)
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
            (archi_context_op_designator_t){.name = instr_init->base.key}, context_value);

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
        const archi_exe_registry_instr_base_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run)
{
    (void) ref_count;

    archi_print_unlock(ARCHI_LOG_VERBOSITY_DEBUG);

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    // Remove the context from the registry, which also decrements the reference count
    archi_status_t code = archi_context_set_slot(registry,
            (archi_context_op_designator_t){.name = instruction->key}, (archi_pointer_t){0});

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
archi_exe_registry_instr_execute_set_value(
        archi_context_t registry,
        const archi_exe_registry_instr_set_value_t *instr_set_value,
        archi_reference_count_t ref_count,
        bool dry_run)
{
    // Print the instruction details
    if (archi_log_verbosity() >= ARCHI_LOG_VERBOSITY_DEBUG)
    {
        {
            PRINT(ARCHI_LOG_INDENT "slot.name = ");

            if (instr_set_value->slot.name != NULL)
                PRINT("\"%s\"\n", instr_set_value->slot.name);
            else
                PRINT("NULL\n");
        }

        if (instr_set_value->slot.num_indices > 0)
        {
            PRINT(ARCHI_LOG_INDENT "slot.indices[%zu] =", instr_set_value->slot.num_indices);

            for (size_t i = 0; i < instr_set_value->slot.num_indices; i++)
                PRINT(" %zu", instr_set_value->slot.index[i]);

            PRINT("\n");
        }

        {
            PRINT(ARCHI_LOG_INDENT "value = ");
            archi_print_value(ARCHI_LOG_INDENT "  ", instr_set_value->value);
        }
    }

    archi_print_unlock(ARCHI_LOG_VERBOSITY_DEBUG);

    if (dry_run)
        return 0;

    if ((instr_set_value->base.key == NULL) || (instr_set_value->base.key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_set_value->base.key}, &code);

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
    archi_pointer_t value = instr_set_value->value;
    value.ref_count = ref_count;

    code = archi_context_set_slot(context_value.ptr, instr_set_value->slot, value);

    if (code != 0)
        return ARCHI_STATUS_TO_ERROR(code);

    return 0;
}

static
archi_status_t
archi_exe_registry_instr_execute_set_context(
        archi_context_t registry,
        const archi_exe_registry_instr_set_context_t *instr_set_context,
        archi_reference_count_t ref_count,
        bool dry_run)
{
    (void) ref_count;

    // Print the instruction details
    if (archi_log_verbosity() >= ARCHI_LOG_VERBOSITY_DEBUG)
    {
        {
            PRINT(ARCHI_LOG_INDENT "slot.name = ");

            if (instr_set_context->slot.name != NULL)
                PRINT("\"%s\"\n", instr_set_context->slot.name);
            else
                PRINT("NULL\n");
        }

        if (instr_set_context->slot.num_indices > 0)
        {
            PRINT(ARCHI_LOG_INDENT "slot.indices[%zu] =", instr_set_context->slot.num_indices);

            for (size_t i = 0; i < instr_set_context->slot.num_indices; i++)
                PRINT(" %zu", instr_set_context->slot.index[i]);

            PRINT("\n");
        }

        {
            PRINT(ARCHI_LOG_INDENT "source_key = ");

            if (instr_set_context->source_key != NULL)
                PRINT("\"%s\"\n", instr_set_context->source_key);
            else
                PRINT("NULL\n");
        }
    }

    archi_print_unlock(ARCHI_LOG_VERBOSITY_DEBUG);

    if (dry_run)
        return 0;

    if ((instr_set_context->base.key == NULL) || (instr_set_context->base.key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;
    else if ((instr_set_context->source_key == NULL) || (instr_set_context->source_key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_set_context->base.key}, &code);

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
            (archi_context_op_designator_t){.name = instr_set_context->source_key}, &code);

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
    code = archi_context_set_slot(context_value.ptr, instr_set_context->slot,
            archi_context_data(src_context_value.ptr));

    if (code != 0)
        return ARCHI_STATUS_TO_ERROR(code);

    return 0;
}

static
archi_status_t
archi_exe_registry_instr_execute_set_slot(
        archi_context_t registry,
        const archi_exe_registry_instr_set_slot_t *instr_set_slot,
        archi_reference_count_t ref_count,
        bool dry_run)
{
    (void) ref_count;

    // Print the instruction details
    if (archi_log_verbosity() >= ARCHI_LOG_VERBOSITY_DEBUG)
    {
        {
            PRINT(ARCHI_LOG_INDENT "slot.name = ");

            if (instr_set_slot->slot.name != NULL)
                PRINT("\"%s\"\n", instr_set_slot->slot.name);
            else
                PRINT("NULL\n");
        }

        if (instr_set_slot->slot.num_indices > 0)
        {
            PRINT(ARCHI_LOG_INDENT "slot.indices[%zu] =", instr_set_slot->slot.num_indices);

            for (size_t i = 0; i < instr_set_slot->slot.num_indices; i++)
                PRINT(" %zu", instr_set_slot->slot.index[i]);

            PRINT("\n");
        }

        {
            PRINT(ARCHI_LOG_INDENT "source_key = ");

            if (instr_set_slot->source_key != NULL)
                PRINT("\"%s\"\n", instr_set_slot->source_key);
            else
                PRINT("NULL\n");
        }

        {
            PRINT(ARCHI_LOG_INDENT "source_slot.name = ");

            if (instr_set_slot->source_slot.name != NULL)
                PRINT("\"%s\"\n", instr_set_slot->source_slot.name);
            else
                PRINT("NULL\n");
        }

        if (instr_set_slot->source_slot.num_indices > 0)
        {
            PRINT(ARCHI_LOG_INDENT "source_slot.indices[%zu] =",
                    instr_set_slot->source_slot.num_indices);

            for (size_t i = 0; i < instr_set_slot->source_slot.num_indices; i++)
                PRINT(" %zu", instr_set_slot->source_slot.index[i]);

            PRINT("\n");
        }
    }

    archi_print_unlock(ARCHI_LOG_VERBOSITY_DEBUG);

    if (dry_run)
        return 0;

    if ((instr_set_slot->base.key == NULL) || (instr_set_slot->base.key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;
    else if ((instr_set_slot->source_key == NULL) || (instr_set_slot->source_key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_set_slot->base.key}, &code);

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
            (archi_context_op_designator_t){.name = instr_set_slot->source_key}, &code);

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
    code = archi_context_copy_slot(context_value.ptr, instr_set_slot->slot,
            src_context_value.ptr, instr_set_slot->source_slot);

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
        bool dry_run)
{
    // Print the instruction details
    if (archi_log_verbosity() >= ARCHI_LOG_VERBOSITY_DEBUG)
    {
        {
            PRINT(ARCHI_LOG_INDENT "action.name = ");

            if (instr_act->action.name != NULL)
                PRINT("\"%s\"\n", instr_act->action.name);
            else
                PRINT("NULL\n");
        }

        if (instr_act->action.num_indices > 0)
        {
            PRINT(ARCHI_LOG_INDENT "action.indices[%zu] =", instr_act->action.num_indices);

            for (size_t i = 0; i < instr_act->action.num_indices; i++)
                PRINT(" %zu", instr_act->action.index[i]);

            PRINT("\n");
        }

        {
            PRINT(ARCHI_LOG_INDENT "dparams_key = ");

            if (instr_act->dparams_key != NULL)
                PRINT("\"%s\"\n", instr_act->dparams_key);
            else
                PRINT("NULL\n");
        }

        {
            PRINT(ARCHI_LOG_INDENT "sparams:\n");

            for (const archi_parameter_list_t *params = instr_act->sparams;
                    params != NULL; params = params->next)
            {
                PRINT(ARCHI_LOG_INDENT "  %s = ", params->name);
                archi_print_value(ARCHI_LOG_INDENT "    ", params->value);
            }
        }
    }

    archi_print_unlock(ARCHI_LOG_VERBOSITY_DEBUG);

    if (dry_run)
        return 0;

    if ((instr_act->base.key == NULL) || (instr_act->base.key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_context_get_slot(registry,
            (archi_context_op_designator_t){.name = instr_act->base.key}, &code);

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
    if (params == NULL)
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

    archi_print_lock(ARCHI_LOG_VERBOSITY_DEBUG);

    if ((instruction == NULL) || (instruction->type == ARCHI_EXE_REGISTRY_INSTR_NOOP))
    {
        PRINT(ARCHI_LOG_INDENT "instruction(NOOP)\n");
        return 0;
    }

#define PRINT_INSTRUCTION_BASE(type) do {                           \
        PRINT(ARCHI_LOG_INDENT "instruction(" #type ") key = ");    \
        if (instruction->key != NULL)                               \
            PRINT("\"%s\"\n", instruction->key);                    \
        else                                                        \
            PRINT("NULL\n");                                        \
    } while (0)

    switch (instruction->type)
    {
        case ARCHI_EXE_REGISTRY_INSTR_INIT:
            PRINT_INSTRUCTION_BASE(INIT);

            return archi_exe_registry_instr_execute_init(registry,
                    (const archi_exe_registry_instr_init_t*)instruction, ref_count, dry_run);

        case ARCHI_EXE_REGISTRY_INSTR_FINAL:
            PRINT_INSTRUCTION_BASE(FINAL);

            return archi_exe_registry_instr_execute_final(registry,
                    instruction, ref_count, dry_run);

        case ARCHI_EXE_REGISTRY_INSTR_SET_VALUE:
            PRINT_INSTRUCTION_BASE(SET_VALUE);

            return archi_exe_registry_instr_execute_set_value(registry,
                    (const archi_exe_registry_instr_set_value_t*)instruction, ref_count, dry_run);

        case ARCHI_EXE_REGISTRY_INSTR_SET_CONTEXT:
            PRINT_INSTRUCTION_BASE(SET_CONTEXT);

            return archi_exe_registry_instr_execute_set_context(registry,
                    (const archi_exe_registry_instr_set_context_t*)instruction, ref_count, dry_run);

        case ARCHI_EXE_REGISTRY_INSTR_SET_SLOT:
            PRINT_INSTRUCTION_BASE(SET_SLOT);

            return archi_exe_registry_instr_execute_set_slot(registry,
                    (const archi_exe_registry_instr_set_slot_t*)instruction, ref_count, dry_run);

        case ARCHI_EXE_REGISTRY_INSTR_ACT:
            PRINT_INSTRUCTION_BASE(ACT);

            return archi_exe_registry_instr_execute_act(registry,
                    (const archi_exe_registry_instr_act_t*)instruction, ref_count, dry_run);

        default:
            PRINT_INSTRUCTION_BASE(<unknown>);

            return !dry_run ? ARCHI_STATUS_EMISUSE : 0;
    }

#undef PRINT_INSTRUCTION_BASE
}

