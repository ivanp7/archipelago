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
#include "archi/ctx/interface/array.var.h"
#include "archi/log/print.fun.h"
#include "archi/log/verbosity.def.h"
#include "archi/log/color.def.h"
#include "archi/util/alloc.fun.h"
#include "archi/util/size.def.h"

#include <stdlib.h> // for malloc(), free()
#include <string.h> // for memcpy()
#include <stdbool.h>

#define MAX_BYTES       (1 << 5)

#define MAX_CHARS       (MAX_BYTES / sizeof(char))
#define MAX_SHORTS      (MAX_BYTES / sizeof(short))
#define MAX_INTS        (MAX_BYTES / sizeof(int))
#define MAX_LONGS       (MAX_BYTES / sizeof(long))
#define MAX_LONGLONGS   (MAX_BYTES / sizeof(long long))
#define MAX_FLOATS      (MAX_BYTES / sizeof(float))
#define MAX_DOUBLES     (MAX_BYTES / sizeof(double))
#define MAX_LONGDOUBLES (MAX_BYTES / sizeof(long double))

#define LOG_INDENT " "

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

        case ARCHI_EXE_REGISTRY_INSTR_INIT_POINTER:
            return sizeof(archi_exe_registry_instr_init_pointer_t);

        case ARCHI_EXE_REGISTRY_INSTR_INIT_DATA_ARRAY:
        case ARCHI_EXE_REGISTRY_INSTR_INIT_FUNC_ARRAY:
            return sizeof(archi_exe_registry_instr_init_array_t);

        case ARCHI_EXE_REGISTRY_INSTR_COPY:
            return sizeof(archi_exe_registry_instr_copy_t);

        case ARCHI_EXE_REGISTRY_INSTR_DELETE:
            return sizeof(archi_exe_registry_instr_delete_t);

        case ARCHI_EXE_REGISTRY_INSTR_SET_TO_VALUE:
            return sizeof(archi_exe_registry_instr_set_to_value_t);

        case ARCHI_EXE_REGISTRY_INSTR_SET_TO_CONTEXT_DATA:
            return sizeof(archi_exe_registry_instr_set_to_context_data_t);

        case ARCHI_EXE_REGISTRY_INSTR_SET_TO_CONTEXT_SLOT:
            return sizeof(archi_exe_registry_instr_set_to_context_slot_t);

        case ARCHI_EXE_REGISTRY_INSTR_ACT:
            return sizeof(archi_exe_registry_instr_act_t);

        default:
            return sizeof(archi_exe_registry_instr_base_t);
    }
}

///////////////////////////////////////////////////////////////////////////////

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
archi_print_key(
        const char *name,
        const char *value)
{
    archi_print(LOG_INDENT "    %s = ", name);

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
    archi_print(LOG_INDENT "    %s.name = ", name);

    if (slot.name != NULL)
        archi_print("\"%s\"\n", slot.name);
    else
        archi_print("NULL\n");

    archi_print(LOG_INDENT "    %s.indices[%zu] =", name, slot.num_indices);

    for (size_t i = 0; i < slot.num_indices; i++)
        archi_print(" %tu", slot.index[i]);

    archi_print("\n");
}

static
void
archi_print_params(
        const char *name,
        const archi_parameter_list_t *params)
{
    if (params != NULL)
    {
        archi_print(LOG_INDENT "    %s:\n", name);

        for (; params != NULL; params = params->next)
        {
            archi_print(LOG_INDENT "      \"%s\": ", params->name);
            archi_print_value(LOG_INDENT "        ", params->value);
        }
    }
    else
        archi_print(LOG_INDENT "    %s: <none>\n", name);
}

///////////////////////////////////////////////////////////////////////////////

struct archi_exe_registry_instr_params {
    archi_parameter_list_t *params;
    archi_parameter_list_t *dparams;
};

static
archi_pointer_t
archi_exe_registry_instr_get_context(
        archi_context_t registry,
        const char *key,
        archi_status_t *code)
{
    archi_pointer_t context_value = archi_context_get_slot(
            registry, (archi_context_slot_t){.name = key}, code);

    if (*code != 1)
        *code = ARCHI_STATUS_TO_ERROR(*code);

    if ((*code == 0) && ((context_value.flags & ARCHI_POINTER_FLAG_FUNCTION) ||
                (context_value.ptr == NULL)))
        *code = ARCHI_STATUS_EVALUE;

    return context_value;
}

static
archi_status_t
archi_exe_registry_instr_check_context(
        archi_context_t registry,
        const char *key)
{
    archi_status_t code;
    archi_exe_registry_instr_get_context(registry, key, &code);

    if (code == 1) // key is not in the registry
        return 0;
    else if (code == 0) // key is in the registry
        return 2;
    else
        return code;
}

static
archi_status_t
archi_exe_registry_instr_add_context(
        archi_context_t registry,
        const char *key,
        archi_pointer_t interface_value,
        struct archi_exe_registry_instr_params params)
{
    archi_status_t code;

    // Initialize the context
    archi_context_t context = archi_context_initialize(interface_value, params.params, &code);
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
    code = archi_context_set_slot(registry, (archi_context_slot_t){.name = key}, context_value);
    if (code != 0)
    {
        archi_context_finalize(context);
        return ARCHI_STATUS_TO_ERROR(code);
    }

    // Decrement the reference count back to 1, making registry the exclusive owner of the context
    archi_reference_count_decrement(context_value.ref_count);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

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

        free((char*)node->name);
        free(node);

        node = next;
    }
}

static
archi_parameter_list_t*
archi_exe_registry_instr_params_alloc(
        archi_parameter_list_t *dparams,
        const archi_parameter_list_t *sparams,
        archi_reference_count_t ref_count,
        archi_status_t *code)
{
    archi_parameter_list_t *head = NULL, *tail = NULL;

    for (const archi_parameter_list_t *params = sparams; params != NULL; params = params->next)
    {
        archi_parameter_list_t *node = malloc(sizeof(*node));
        if (node == NULL)
        {
            archi_exe_registry_instr_params_free(head, NULL);
            *code = ARCHI_STATUS_ENOMEMORY;
            return NULL;
        }

        char *name = archi_copy_string(params->name);
        if (name == NULL)
        {
            free(node);
            archi_exe_registry_instr_params_free(head, NULL);
            *code = ARCHI_STATUS_ENOMEMORY;
            return NULL;
        }

        *node = (archi_parameter_list_t){
            .name = name,
            .value = params->value,
        };
        if (params->value.ptr != NULL)
            node->value.ref_count = ref_count;

        if (tail != NULL)
        {
            tail->next = node;
            tail = node;
        }
        else
            head = tail = node;
    }

    *code = 0;

    if (tail != NULL)
    {
        tail->next = dparams;
        return head;
    }
    else
        return dparams;
}

static
struct archi_exe_registry_instr_params
archi_exe_registry_instr_prepare_params(
        archi_context_t registry,
        const char *dparams_key,
        const archi_parameter_list_t *sparams,
        archi_reference_count_t ref_count,
        archi_status_t *code)
{
    struct archi_exe_registry_instr_params params = {0};

    if (dparams_key != NULL)
    {
        archi_pointer_t dparams_value = archi_exe_registry_instr_get_context(
                registry, dparams_key, code);
        if (*code != 0)
            return params;

        params.dparams = archi_context_data(dparams_value.ptr).ptr;
    }

    params.params = archi_exe_registry_instr_params_alloc(params.dparams, sparams, ref_count, code);

    return params;
}

///////////////////////////////////////////////////////////////////////////////

static
archi_status_t
archi_exe_registry_instr_execute_init_from_context(
        archi_context_t registry,
        const archi_exe_registry_instr_init_from_context_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);
        archi_print_key("interface_source_key", instruction->interface_source_key);
        archi_print_key("dparams_key", instruction->dparams_key);
        archi_print_params("sparams", instruction->sparams);

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Check early if the context key exists already
    code = archi_exe_registry_instr_check_context(registry, instruction->key);
    if (code != 0)
        return code;

    // Obtain the context interface
    archi_pointer_t interface_value;

    if (instruction->interface_source_key != NULL)
    {
        // Obtain the source context from the registry
        archi_pointer_t src_context_value = archi_exe_registry_instr_get_context(
                registry, instruction->interface_source_key, &code);
        if (code != 0)
            return code;

        // Get context interface from the source context
        interface_value = archi_context_interface(src_context_value.ptr);
    }
    else // parameter list
    {
        interface_value = (archi_pointer_t){
            .ptr = (void*)&archi_context_parameters_interface,
            .element = {
                .num_of = 1,
            },
        };
    }

    // Prepare the context initialization parameter list
    struct archi_exe_registry_instr_params params = archi_exe_registry_instr_prepare_params(
            registry, instruction->dparams_key, instruction->sparams, ref_count, &code);
    if (code != 0)
        return code;

    // Initialize the context and add it to the registry
    code = archi_exe_registry_instr_add_context(registry, instruction->key, interface_value, params);

    archi_exe_registry_instr_params_free(params.params, params.dparams);

    return code;
}

static
archi_status_t
archi_exe_registry_instr_execute_init_from_slot(
        archi_context_t registry,
        const archi_exe_registry_instr_init_from_slot_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);
        archi_print_key("interface_source_key", instruction->interface_source_key);
        archi_print_slot("interface_source_slot", instruction->interface_source_slot);
        archi_print_key("dparams_key", instruction->dparams_key);
        archi_print_params("sparams", instruction->sparams);

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Check early if the context key exists already
    code = archi_exe_registry_instr_check_context(registry, instruction->key);
    if (code != 0)
        return code;

    // Obtain the context interface
    archi_pointer_t interface_value;
    {
        // Obtain the source context from the registry
        archi_pointer_t src_context_value = archi_exe_registry_instr_get_context(
                registry, instruction->interface_source_key, &code);
        if (code != 0)
            return code;

        // Get context interface from the source context slot
        interface_value = archi_context_get_slot(src_context_value.ptr,
                instruction->interface_source_slot, &code);

        if (code != 0)
            return ARCHI_STATUS_TO_ERROR(code);
    }

    // Prepare the context initialization parameter list
    struct archi_exe_registry_instr_params params = archi_exe_registry_instr_prepare_params(
            registry, instruction->dparams_key, instruction->sparams, ref_count, &code);
    if (code != 0)
        return code;

    // Initialize the context and add it to the registry
    code = archi_exe_registry_instr_add_context(registry, instruction->key, interface_value, params);

    archi_exe_registry_instr_params_free(params.params, params.dparams);

    return code;
}

static
archi_status_t
archi_exe_registry_instr_execute_init_pointer(
        archi_context_t registry,
        const archi_exe_registry_instr_init_pointer_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);
        {
            archi_print(LOG_INDENT "    value = ");
            archi_print_value(LOG_INDENT "      ", instruction->value);
        }

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Check early if the context key exists already
    code = archi_exe_registry_instr_check_context(registry, instruction->key);
    if (code != 0)
        return code;

    // Prepare the context interface
    archi_pointer_t interface_value = (archi_pointer_t){
        .ptr = (void*)&archi_context_pointer_interface,
        .element = {
            .num_of = 1,
        },
    };

    // Prepare the context initialization parameter list
    archi_parameter_list_t params_node[] = {
        {
            .name = "value",
            .value = instruction->value,
        },
    };
    if (instruction->value.ptr != NULL)
        params_node[0].value.ref_count = ref_count;

    struct archi_exe_registry_instr_params params = {.params = params_node};

    // Initialize the context and add it to the registry
    code = archi_exe_registry_instr_add_context(registry, instruction->key, interface_value, params);

    return code;
}

static
archi_status_t
archi_exe_registry_instr_execute_init_array(
        archi_context_t registry,
        const archi_exe_registry_instr_init_array_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    (void) ref_count;

    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);
        {
            archi_print(LOG_INDENT "    num_elements = %zu\n", instruction->num_elements);
            archi_print(LOG_INDENT "    flags = %llX\n", (unsigned long long)instruction->flags);
            archi_print(LOG_INDENT "    func_ptrs = %s\n",
                    (instruction->base.type == ARCHI_EXE_REGISTRY_INSTR_INIT_FUNC_ARRAY) ? "true" : "false");
        }

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Check early if the context key exists already
    code = archi_exe_registry_instr_check_context(registry, instruction->key);
    if (code != 0)
        return code;

    // Prepare the context interface
    archi_pointer_t interface_value = (archi_pointer_t){
        .ptr = (void*)&archi_context_array_interface,
        .element = {
            .num_of = 1,
        },
    };

    // Prepare the context initialization parameter list
    char func_ptrs = (instruction->base.type == ARCHI_EXE_REGISTRY_INSTR_INIT_FUNC_ARRAY) ? 1 : 0;

    archi_parameter_list_t params_node[] = {
        {
            .name = "num_elements",
            .value = (archi_pointer_t){.ptr = (void*)&instruction->num_elements},
        },
        {
            .name = "flags",
            .value = (archi_pointer_t){.ptr = (void*)&instruction->flags},
        },
        {
            .name = "func_ptrs",
            .value = (archi_pointer_t){.ptr = (void*)&func_ptrs},
        },
    };
    params_node[0].next = &params_node[1];
    params_node[1].next = &params_node[2];

    struct archi_exe_registry_instr_params params = {.params = params_node};

    // Initialize the context and add it to the registry
    code = archi_exe_registry_instr_add_context(registry, instruction->key, interface_value, params);

    return code;
}

static
archi_status_t
archi_exe_registry_instr_execute_copy(
        archi_context_t registry,
        const archi_exe_registry_instr_copy_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    (void) ref_count;

    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);
        archi_print_key("original_key", instruction->original_key);

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;
    else if ((instruction->original_key == NULL) || (instruction->original_key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_exe_registry_instr_get_context(
            registry, instruction->original_key, &code);
    if (code != 0)
        return code;

    // Insert the context to the registry, which also increments the reference count
    code = archi_context_set_slot(registry,
            (archi_context_slot_t){.name = instruction->key}, context_value);
    code = ARCHI_STATUS_TO_ERROR(code);

    return code;
}

static
archi_status_t
archi_exe_registry_instr_execute_delete(
        archi_context_t registry,
        const archi_exe_registry_instr_delete_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    (void) ref_count;

    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    // Remove the context from the registry, which also decrements the reference count
    archi_status_t code = archi_context_set_slot(registry,
            (archi_context_slot_t){.name = instruction->key}, (archi_pointer_t){0});
    if (code != 1)
        code = ARCHI_STATUS_TO_ERROR(code);

    return code;
}

static
archi_status_t
archi_exe_registry_instr_execute_set_to_value(
        archi_context_t registry,
        const archi_exe_registry_instr_set_to_value_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);
        archi_print_slot("slot", instruction->slot);
        {
            archi_print(LOG_INDENT "    value = ");
            archi_print_value(LOG_INDENT "      ", instruction->value);
        }

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_exe_registry_instr_get_context(
            registry, instruction->key, &code);
    if (code != 0)
        return code;

    // Set the context value
    archi_pointer_t value = instruction->value;
    if (instruction->value.ptr != NULL)
        value.ref_count = ref_count;

    code = archi_context_set_slot(context_value.ptr, instruction->slot, value);
    code = ARCHI_STATUS_TO_ERROR(code);

    return code;
}

static
archi_status_t
archi_exe_registry_instr_execute_set_to_context_data(
        archi_context_t registry,
        const archi_exe_registry_instr_set_to_context_data_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    (void) ref_count;

    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);
        archi_print_slot("slot", instruction->slot);
        archi_print_key("source_key", instruction->source_key);

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;
    else if ((instruction->source_key == NULL) || (instruction->source_key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_exe_registry_instr_get_context(
            registry, instruction->key, &code);
    if (code != 0)
        return code;

    // Obtain the source context from the registry
    archi_pointer_t src_context_value = archi_exe_registry_instr_get_context(
            registry, instruction->source_key, &code);
    if (code != 0)
        return code;

    // Set the context value
    code = archi_context_set_slot(context_value.ptr, instruction->slot,
            archi_context_data(src_context_value.ptr));
    code = ARCHI_STATUS_TO_ERROR(code);

    return code;
}

static
archi_status_t
archi_exe_registry_instr_execute_set_to_context_slot(
        archi_context_t registry,
        const archi_exe_registry_instr_set_to_context_slot_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    (void) ref_count;

    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);
        archi_print_slot("slot", instruction->slot);
        archi_print_key("source_key", instruction->source_key);
        archi_print_slot("source_slot", instruction->source_slot);

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;
    else if ((instruction->source_key == NULL) || (instruction->source_key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_exe_registry_instr_get_context(
            registry, instruction->key, &code);
    if (code != 0)
        return code;

    // Obtain the source context from the registry
    archi_pointer_t src_context_value = archi_exe_registry_instr_get_context(
            registry, instruction->source_key, &code);
    if (code != 0)
        return code;

    // Set the context value
    code = archi_context_copy_slot(context_value.ptr, instruction->slot,
            src_context_value.ptr, instruction->source_slot);
    code = ARCHI_STATUS_TO_ERROR(code);

    return code;
}

static
archi_status_t
archi_exe_registry_instr_execute_act(
        archi_context_t registry,
        const archi_exe_registry_instr_act_t *instruction,
        archi_reference_count_t ref_count,
        bool dry_run,
        bool logging)
{
    // Print the instruction details
    if (logging)
    {
        archi_print_key("key", instruction->key);
        archi_print_slot("action", instruction->action);
        archi_print_key("dparams_key", instruction->dparams_key);
        archi_print_params("sparams", instruction->sparams);

        archi_print_color(ARCHI_COLOR_RESET "\n");
        archi_print_unlock();
    }

    if (dry_run)
        return 0;

    if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    archi_status_t code;

    // Obtain the context from the registry
    archi_pointer_t context_value = archi_exe_registry_instr_get_context(
            registry, instruction->key, &code);
    if (code != 0)
        return code;

    // Prepare the context action parameters
    struct archi_exe_registry_instr_params params = archi_exe_registry_instr_prepare_params(
            registry, instruction->dparams_key, instruction->sparams, ref_count, &code);
    if (code != 0)
        return code;

    // Invoke the context action
    code = archi_context_act(context_value.ptr, instruction->action, params.params);
    code = ARCHI_STATUS_TO_ERROR(code);

    archi_exe_registry_instr_params_free(params.params, params.dparams);

    return code;
}

///////////////////////////////////////////////////////////////////////////////

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
        archi_print_color(ARCHI_LOG_COLOR_DEBUG);

    if ((instruction == NULL) || (instruction->type == ARCHI_EXE_REGISTRY_INSTR_NOOP))
    {
        if (logging)
        {
            archi_print(" [NOOP]\n");

            archi_print_color(ARCHI_COLOR_RESET "\n");
            archi_print_unlock();
        }

        return 0;
    }

    archi_status_t code;

#define INSTRUCTION(type, name)                                                 \
        case ARCHI_EXE_REGISTRY_INSTR_##type:                                   \
            if (logging)                                                        \
                archi_print(" [" #type "]\n");                                  \
                                                                                \
            code = archi_exe_registry_instr_execute_##name(registry,            \
                    (const archi_exe_registry_instr_##name##_t*)instruction,    \
                    ref_count, dry_run, logging);                               \
            break

    switch (instruction->type)
    {
        INSTRUCTION(INIT_FROM_CONTEXT, init_from_context);
        INSTRUCTION(INIT_FROM_SLOT, init_from_slot);
        INSTRUCTION(INIT_POINTER, init_pointer);
        INSTRUCTION(INIT_DATA_ARRAY, init_array);
        INSTRUCTION(INIT_FUNC_ARRAY, init_array);
        INSTRUCTION(COPY, copy);
        INSTRUCTION(DELETE, delete);
        INSTRUCTION(SET_TO_VALUE, set_to_value);
        INSTRUCTION(SET_TO_CONTEXT_DATA, set_to_context_data);
        INSTRUCTION(SET_TO_CONTEXT_SLOT, set_to_context_slot);
        INSTRUCTION(ACT, act);

        default:
            if (logging)
            {
                archi_print(" <unknown instruction type>\n");

                archi_print_color(ARCHI_COLOR_RESET "\n");
                archi_print_unlock();
            }

            code = !dry_run ? ARCHI_STATUS_EMISUSE : 0;
    }

    return code;
}

