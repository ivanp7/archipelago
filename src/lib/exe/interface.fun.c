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
 * @brief Application context interface.
 */

#include "archi/exe/interface.fun.h"
#include "archi/app/context.fun.h"
#include "archi/fsm/instance.typ.h"
#include "archi/util/error.def.h"
#include "archi/util/os/signal.fun.h"

#include <string.h> // for strcmp()

#define CHECK_STRUCT(struct_type) do { \
    if ((value->type != ARCHI_VALUE_DATA) || (value->ptr == NULL) || (value->num_of == 0) || \
            ((value->size != 0) && (value->size < sizeof(struct_type)))) \
        return ARCHI_ERROR_MISUSE; \
} while (0)

#define CHECK_FUNCTION() do { \
    if (value->type != ARCHI_VALUE_FUNCTION) \
        return ARCHI_ERROR_MISUSE; \
} while (0)

#define CHECK_DATA() do { \
    if (value->type != ARCHI_VALUE_DATA) \
        return ARCHI_ERROR_MISUSE; \
} while (0)

/*****************************************************************************/

ARCHI_CONTEXT_SET_FUNC(archi_app_signal_set)
{
    if ((context == NULL) || (slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    struct archi_signal_management_context *signal_management = context;

    if (strcmp(slot, ARCHI_APP_SIGNAL_SLOT_SIGNAL_HANDLER) == 0)
    {
        CHECK_STRUCT(archi_signal_handler_t);
        archi_signal_management_set_handler(signal_management, *(archi_signal_handler_t*)value->ptr);
    }
    else if (strcmp(slot, ARCHI_APP_SIGNAL_SLOT_SIGNAL_HANDLER_FUNC) == 0)
    {
        CHECK_FUNCTION();
        archi_signal_handler_t signal_handler = archi_signal_management_handler(signal_management);
        signal_handler.function = (archi_signal_handler_function_t)value->fptr;
        archi_signal_management_set_handler(signal_management, signal_handler);
    }
    else if (strcmp(slot, ARCHI_APP_SIGNAL_SLOT_SIGNAL_HANDLER_DATA) == 0)
    {
        CHECK_DATA();
        archi_signal_handler_t signal_handler = archi_signal_management_handler(signal_management);
        signal_handler.data = value->ptr;
        archi_signal_management_set_handler(signal_management, signal_handler);
    }
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

ARCHI_CONTEXT_GET_FUNC(archi_app_signal_get)
{
    if ((context == NULL) || (slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    struct archi_signal_management_context *signal_management = context;

    if (strcmp(slot, ARCHI_APP_SIGNAL_SLOT_SIGNAL_HANDLER_FUNC) == 0)
    {
        *value = (archi_value_t){
            .fptr = (archi_function_t)archi_signal_management_handler(signal_management).function,
            .num_of = 1,
            .type = ARCHI_VALUE_FUNCTION,
        };
    }
    else if (strcmp(slot, ARCHI_APP_SIGNAL_SLOT_SIGNAL_HANDLER_DATA) == 0)
    {
        *value = (archi_value_t){
            .ptr = archi_signal_management_handler(signal_management).data,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_SIGNAL_SLOT_SIGNAL_FLAGS) == 0)
    {
        *value = (archi_value_t){
            .ptr = archi_signal_management_flags(signal_management),
            .size = ARCHI_SIGNAL_FLAGS_SIZEOF,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

const archi_context_interface_t archi_app_signal_interface = {
    .set_fn = archi_app_signal_set,
    .get_fn = archi_app_signal_get,
};

/*****************************************************************************/

ARCHI_CONTEXT_SET_FUNC(archi_app_fsm_set)
{
    if ((context == NULL) || (slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_fsm_t *fsm = context;

    if (strcmp(slot, ARCHI_APP_FSM_SLOT_ENTRY_STATE) == 0)
    {
        CHECK_STRUCT(archi_fsm_state_t);
        memcpy(&fsm->entry_state, value->ptr, sizeof(fsm->entry_state));
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_ENTRY_STATE_FUNC) == 0)
    {
        CHECK_FUNCTION();
        fsm->entry_state.function = (archi_fsm_state_function_t)value->fptr;
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_ENTRY_STATE_DATA) == 0)
    {
        CHECK_DATA();
        fsm->entry_state.data = value->ptr;
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_ENTRY_STATE_METADATA) == 0)
    {
        CHECK_DATA();
        fsm->entry_state.metadata = value->ptr;
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_TRANSITION) == 0)
    {
        CHECK_STRUCT(archi_fsm_transition_t);
        memcpy(&fsm->transition, value->ptr, sizeof(fsm->transition));
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_TRANSITION_FUNC) == 0)
    {
        CHECK_FUNCTION();
        fsm->transition.function = (archi_fsm_transition_function_t)value->fptr;
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_TRANSITION_DATA) == 0)
    {
        CHECK_DATA();
        fsm->transition.data = value->ptr;
    }
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

ARCHI_CONTEXT_GET_FUNC(archi_app_fsm_get)
{
    if ((context == NULL) || (slot == NULL) || (value == NULL))
        return ARCHI_ERROR_MISUSE;

    archi_fsm_t *fsm = context;

    if (strcmp(slot, ARCHI_APP_FSM_SLOT_ENTRY_STATE) == 0)
    {
        *value = (archi_value_t){
            .ptr = &fsm->entry_state,
            .size = sizeof(fsm->entry_state),
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_ENTRY_STATE_FUNC) == 0)
    {
        *value = (archi_value_t){
            .fptr = (archi_function_t)fsm->entry_state.function,
            .num_of = 1,
            .type = ARCHI_VALUE_FUNCTION,
        };
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_ENTRY_STATE_DATA) == 0)
    {
        *value = (archi_value_t){
            .ptr = fsm->entry_state.data,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_ENTRY_STATE_METADATA) == 0)
    {
        *value = (archi_value_t){
            .ptr = fsm->entry_state.metadata,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_TRANSITION) == 0)
    {
        *value = (archi_value_t){
            .ptr = &fsm->transition,
            .size = sizeof(fsm->transition),
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_TRANSITION_FUNC) == 0)
    {
        *value = (archi_value_t){
            .fptr = (archi_function_t)fsm->transition.function,
            .num_of = 1,
            .type = ARCHI_VALUE_FUNCTION,
        };
    }
    else if (strcmp(slot, ARCHI_APP_FSM_SLOT_TRANSITION_DATA) == 0)
    {
        *value = (archi_value_t){
            .ptr = fsm->transition.data,
            .num_of = 1,
            .type = ARCHI_VALUE_DATA,
        };
    }
    else
        return ARCHI_ERROR_CONFIG;

    return 0;
}

const archi_context_interface_t archi_app_fsm_interface = {
    .set_fn = archi_app_fsm_set,
    .get_fn = archi_app_fsm_get,
};

