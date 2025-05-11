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
 * @brief Application configuration instructions.
 */

#include "archi/app/registry.fun.h"
#include "archi/app/registry.typ.h"
#include "archi/app/context.fun.h"
#include "archi/app/context/parameters.var.h"
#include "archi/app/context/pointer.var.h"
#include "archi/util/ref_count.fun.h"

#include <stdlib.h> // for malloc(), free()
#include <stdbool.h>
#include <stdalign.h>

#define INSTRUCTION_POINTER(name, type) const type *name = (const type*)instruction

size_t
archi_context_registry_instr_sizeof(
        const archi_context_registry_instr_base_t *instruction)
{
    if (instruction == NULL)
        return 0;

    switch (instruction->type)
    {
        case ARCHI_CONTEXT_REGISTRY_INSTR_INIT_STATIC:
        case ARCHI_CONTEXT_REGISTRY_INSTR_INIT_DYNAMIC:
            return sizeof(archi_context_registry_instr_init_t);

        case ARCHI_CONTEXT_REGISTRY_INSTR_SET_VALUE:
            return sizeof(archi_context_registry_instr_set_value_t);

        case ARCHI_CONTEXT_REGISTRY_INSTR_SET_CONTEXT:
            return sizeof(archi_context_registry_instr_set_context_t);

        case ARCHI_CONTEXT_REGISTRY_INSTR_SET_SLOT:
            return sizeof(archi_context_registry_instr_set_slot_t);

        case ARCHI_CONTEXT_REGISTRY_INSTR_ACT_STATIC:
        case ARCHI_CONTEXT_REGISTRY_INSTR_ACT_DYNAMIC:
            return sizeof(archi_context_registry_instr_act_t);

        default:
            return sizeof(archi_context_registry_instr_base_t);
    }
}

static
archi_status_t
archi_context_registry_instr_execute_init(
        struct archi_context *registry,
        const archi_context_registry_instr_init_t *instr_init,
        bool dynamic_params)
{
    if (dynamic_params && ((instr_init->dparams_key == NULL) ||
                (instr_init->dparams_key[0] == '\0')))
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
                .size = sizeof(archi_context_parameters_interface),
                .alignment = alignof(archi_context_interface_t),
            },
        };
    else if (instr_init->interface_key[0] == '\0') // pointer copy
        interface_value = (archi_pointer_t){
            .ptr = (void*)&archi_context_pointer_interface,
            .element = {
                .num_of = 1,
                .size = sizeof(archi_context_pointer_interface),
                .alignment = alignof(archi_context_interface_t),
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

    // Obtain the context initialization parameters
    const archi_context_parameter_list_t *params;

    if (!dynamic_params)
        params = instr_init->sparams;
    else
    {
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

        params = dparams_value.ptr;
    }

    // Initialize the context
    struct archi_context *context = archi_context_initialize(interface_value, params, &code);

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
archi_context_registry_instr_execute_final(
        struct archi_context *registry,
        const archi_context_registry_instr_base_t *instruction)
{
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
archi_context_registry_instr_execute_set_value(
        struct archi_context *registry,
        const archi_context_registry_instr_set_value_t *instr_set_value)
{
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
    code = archi_context_set_slot(context_value.ptr, instr_set_value->slot, instr_set_value->value);

    if (code != 0)
        return ARCHI_STATUS_TO_ERROR(code);

    return 0;
}

static
archi_status_t
archi_context_registry_instr_execute_set_context(
        struct archi_context *registry,
        const archi_context_registry_instr_set_context_t *instr_set_context)
{
    if ((instr_set_context->source_key == NULL) || (instr_set_context->source_key[0] == '\0'))
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
archi_context_registry_instr_execute_set_slot(
        struct archi_context *registry,
        const archi_context_registry_instr_set_slot_t *instr_set_slot)
{
    if ((instr_set_slot->source_key == NULL) || (instr_set_slot->source_key[0] == '\0'))
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
archi_context_registry_instr_execute_act(
        struct archi_context *registry,
        const archi_context_registry_instr_act_t *instr_act,
        bool dynamic_params)
{
    if (dynamic_params && ((instr_act->dparams_key == NULL) ||
                (instr_act->dparams_key[0] == '\0')))
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
    const archi_context_parameter_list_t *params;

    if (!dynamic_params)
        params = instr_act->sparams;
    else
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

        params = dparams_value.ptr;
    }

    // Invoke the context action
    code = archi_context_act(context_value.ptr, instr_act->action, params);

    if (code != 0)
        return ARCHI_STATUS_TO_ERROR(code);

    return 0;
}

archi_status_t
archi_context_registry_instr_execute(
        struct archi_context *registry,
        const archi_context_registry_instr_base_t *instruction)
{
    if ((registry == NULL) || (instruction == NULL))
        return ARCHI_STATUS_EMISUSE;

    if (instruction->type == ARCHI_CONTEXT_REGISTRY_INSTR_NOOP)
        return 0;

    if (instruction->type == ARCHI_CONTEXT_REGISTRY_INSTR_HALT)
        return ARCHI_STATUS_EMISUSE;
    else if ((instruction->key == NULL) || (instruction->key[0] == '\0'))
        return ARCHI_STATUS_EMISUSE;

    switch (instruction->type)
    {
        case ARCHI_CONTEXT_REGISTRY_INSTR_INIT_STATIC:
        case ARCHI_CONTEXT_REGISTRY_INSTR_INIT_DYNAMIC:
            return archi_context_registry_instr_execute_init(registry,
                    (const archi_context_registry_instr_init_t*)instruction,
                    instruction->type == ARCHI_CONTEXT_REGISTRY_INSTR_INIT_DYNAMIC);

        case ARCHI_CONTEXT_REGISTRY_INSTR_FINAL:
            return archi_context_registry_instr_execute_final(registry,
                    instruction);

        case ARCHI_CONTEXT_REGISTRY_INSTR_SET_VALUE:
            return archi_context_registry_instr_execute_set_value(registry,
                    (const archi_context_registry_instr_set_value_t*)instruction);

        case ARCHI_CONTEXT_REGISTRY_INSTR_SET_CONTEXT:
            return archi_context_registry_instr_execute_set_context(registry,
                    (const archi_context_registry_instr_set_context_t*)instruction);

        case ARCHI_CONTEXT_REGISTRY_INSTR_SET_SLOT:
            return archi_context_registry_instr_execute_set_slot(registry,
                    (const archi_context_registry_instr_set_slot_t*)instruction);

        case ARCHI_CONTEXT_REGISTRY_INSTR_ACT_STATIC:
        case ARCHI_CONTEXT_REGISTRY_INSTR_ACT_DYNAMIC:
            return archi_context_registry_instr_execute_act(registry,
                    (const archi_context_registry_instr_act_t*)instruction,
                    instruction->type == ARCHI_CONTEXT_REGISTRY_INSTR_ACT_DYNAMIC);

        default:
            return ARCHI_STATUS_EMISUSE;
    }
}

