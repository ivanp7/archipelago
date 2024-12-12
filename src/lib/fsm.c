/*****************************************************************************
 * Copyright (C) 2023-2024 by Ivan Podmazov                                  *
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
 * @brief Finite state machines implementation.
 */

#include "archi/fsm.h"
#include "archi/util/error.def.h"

#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>

#define ARCHI_FSM_INITIAL_STACK_CAPACITY 32

enum {
    J_STATE = 0,
    J_TRANSITION,
};

struct archi_finite_state_machine_context {
    archi_state_t current_state;
    archi_state_transition_t state_transition;

    archi_state_t *stack;
    size_t stack_size;
    size_t stack_capacity;

    archi_status_t code;

    bool in_state_function;
    jmp_buf env; // non-local jumps
};

static
bool
archi_state_context_stack_reserve(
        struct archi_finite_state_machine_context *context,
        size_t size)
{
    if (context->stack_capacity >= context->stack_size + size)
        return true; // the current capacity is enough

    size_t new_stack_capacity = context->stack_capacity;
    do
        new_stack_capacity *= 2; // double the stack capacity
    while (new_stack_capacity < context->stack_size + size);

    archi_state_t *new_stack = realloc(context->stack,
            sizeof(*context->stack) * new_stack_capacity);
    if (new_stack == NULL)
        return false;

    context->stack = new_stack;
    context->stack_capacity = new_stack_capacity;

    return true;
}

/*****************************************************************************/

static
void
archi_finite_state_machine_loop(
        struct archi_finite_state_machine_context *context);

static
bool
archi_finite_state_machine_transition(
        struct archi_finite_state_machine_context *context);

archi_status_t
archi_finite_state_machine(
        archi_state_t entry_state,
        archi_state_transition_t state_transition)
{
    // Process the degenerate case
    if ((entry_state.function == NULL) && (state_transition.function == NULL))
        return 0;

    // Initialize the context
    struct archi_finite_state_machine_context context = {
        .state_transition = state_transition,

        .stack_size = 1,
        .stack_capacity = ARCHI_FSM_INITIAL_STACK_CAPACITY,
    };

    // Allocate the stack memory
    context.stack = malloc(sizeof(*context.stack) * context.stack_capacity);
    if (context.stack == NULL)
        return ARCHI_ERROR_ALLOC;

    context.stack[0] = entry_state;

    // Run the finite state machine loop
    /****************************************/
    archi_finite_state_machine_loop(&context);
    /****************************************/

    // Free the stack memory and return
    free(context.stack);

    return context.code;
}

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wimplicit-fallthrough" // make gcc/clang shut up about the switch not having any breaks
#endif

void
archi_finite_state_machine_loop(
        struct archi_finite_state_machine_context *context)
{
    while (archi_finite_state_machine_transition(context)) // there is a next state
    {
        // Set the jump point, call the current state function and process its response after return/jump
        switch (setjmp(context->env))
        {
            case J_STATE: // no jump were performed yet, call the state function
                context->in_state_function = true;
                {
                    /***************************************/
                    context->current_state.function(context);
                    /***************************************/
                }
            case J_TRANSITION: // returned from the state function, perform transition to the next state
                context->in_state_function = false;
                continue;

            default: // shouldn't happen
                abort();
        }
    }
}

#ifdef __GNUC__
#  pragma GCC diagnostic pop // ignored "-Wimplicit-fallthrough"
#endif

bool
archi_finite_state_machine_transition(
        struct archi_finite_state_machine_context *context)
{
    if (context->state_transition.function == NULL)
    {
        if (context->stack_size > 0) // pop the next state from the stack
        {
            context->current_state = context->stack[--context->stack_size];
            return true;
        }
        else // the stack is empty, exit now
            return false;
    }

    archi_state_t stack_top, trans_state = {0};

    if (context->stack_size > 0) // the next state is at the stack top
        stack_top = context->stack[context->stack_size - 1];
    else
        stack_top = ARCHI_NULL_STATE;

    // Call the state transition function

    /********************************************************************/
    context->state_transition.function(context->current_state, stack_top,
            &trans_state, &context->code, context->state_transition.data);
    /********************************************************************/

    // Update the current state
    if (trans_state.function != NULL)
        context->current_state = trans_state;
    else
    {
        context->current_state = stack_top;

        if (context->stack_size > 0) // pop the next state from the stack
            context->stack_size--;
        else // the stack is empty, exit now
            return false;
    }

    return true;
}

/*****************************************************************************/

archi_state_t
archi_current(
        const struct archi_finite_state_machine_context *context)
{
    return (context != NULL) ? context->current_state : ARCHI_NULL_STATE;
}

size_t
archi_stack_size(
        const struct archi_finite_state_machine_context *context)
{
    return (context != NULL) ? context->stack_size : 0;
}

archi_status_t
archi_code(
        const struct archi_finite_state_machine_context *context)
{
    return (context != NULL) ? context->code : 0;
}

void
archi_set_code(
        struct archi_finite_state_machine_context *context,

        archi_status_t code)
{
    if ((context == NULL) || !context->in_state_function)
        return;

    context->code = code;
}

/*****************************************************************************/

static
void
archi_done(
        struct archi_finite_state_machine_context *context)
{
    longjmp(context->env, J_TRANSITION);
}

static
void
archi_error(
        struct archi_finite_state_machine_context *context,
        archi_status_t code)
{
    archi_set_code(context, code);
    context->stack_size = 0;
    archi_done(context);
}

void
archi_proceed(
        struct archi_finite_state_machine_context *context,

        size_t num_popped,
        size_t num_pushed,
        const archi_state_t pushed[])
{
    if ((context == NULL) || !context->in_state_function)
        return;
    else if (num_popped > context->stack_size)
        archi_error(context, ARCHI_ERROR_MISUSE);
    else if ((num_pushed > 0) && (pushed == NULL))
        archi_error(context, ARCHI_ERROR_MISUSE);

    // Pop states from the stack
    context->stack_size -= num_popped;

    // Push states to the stack
    for (size_t i = 0; i < num_pushed; i++)
    {
        size_t index = (num_pushed-1) - i; // reverse order
        if (pushed[index].function == NULL)
            continue;

        // Reserve a seat in the stack
        if (!archi_state_context_stack_reserve(context, 1))
            archi_error(context, ARCHI_ERROR_ALLOC);

        // Push
        context->stack[context->stack_size++] = pushed[index];
    }

    // Proceed
    archi_done(context);
}

/*****************************************************************************/

ARCHI_STATE_FUNCTION(archi_state_chain_execute)
{
    archi_state_chain_t *chain = ARCHI_STATE_DATA(archi_state_chain_t);
    if (chain == NULL)
        ARCHI_DONE(0);

    archi_state_t after_next_state = {.data = chain->data};
    if (after_next_state.data == NULL)
        ARCHI_DONE(0); // optimization saving a state

    after_next_state.function = archi_state_chain_execute;
    ARCHI_PROCEED(0, chain->next_state, after_next_state);
}

