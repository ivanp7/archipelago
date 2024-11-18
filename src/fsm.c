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
 * @brief Finite state machine implementation.
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
    J_EXIT,
};

struct archi_state_context {
    // State registers
    archi_state_t current_state;
    archi_state_t next_state;
    archi_state_t return_state;

    // Input state transition
    archi_state_transition_t state_transition;

    // State stack
    archi_state_t *stack;
    size_t stack_size;
    size_t stack_capacity;

    // Miscellaneous
    archi_status_t exit_code;

    jmp_buf env;
};

static
bool
archi_state_context_stack_push(
        struct archi_state_context *context,
        archi_state_t state)
{
    // Check if the stack is full
    if (context->stack_size == context->stack_capacity)
    {
        // Increase the stack capacity
        size_t new_stack_capacity = context->stack_capacity * 2;
        if (new_stack_capacity / 2 != context->stack_capacity) // check for integer overflow
            return false;

        archi_state_t *new_stack = realloc(context->stack,
                sizeof(*context->stack) * new_stack_capacity);
        if (new_stack == NULL)
            return false;

        context->stack = new_stack;
        context->stack_capacity = new_stack_capacity;
    }

    // Push the stack
    context->stack[context->stack_size++] = state;
    return true;
}

static
bool
archi_state_context_stack_pop(
        struct archi_state_context *context,
        archi_state_t *state)
{
    if (context->stack_size > 0)
    {
        *state = context->stack[--context->stack_size];
        return true;
    }
    else
        return false;
}

/*****************************************************************************/

static
void
archi_finite_state_machine_loop(
        struct archi_state_context *context);

static
bool
archi_finite_state_machine_state_transition(
        struct archi_state_context *context);

archi_status_t
archi_finite_state_machine(
        archi_state_t entry_state,
        archi_state_transition_t state_transition)
{
    // Process the degenerate case
    if ((entry_state.function == NULL) && (state_transition.function == NULL))
        return 0;

    // Initialize the context
    struct archi_state_context context = {
        .next_state = entry_state,
        .state_transition = state_transition,

        .stack_capacity = ARCHI_FSM_INITIAL_STACK_CAPACITY,
    };

    // Allocate the stack memory
    context.stack = malloc(sizeof(*context.stack) * context.stack_capacity);
    if (context.stack == NULL)
        return ARCHI_ERROR_ALLOC;

    // Run the finite state machine loop
    /****************************************/
    archi_finite_state_machine_loop(&context);
    /****************************************/

    // Free the stack memory and return
    free(context.stack);

    return context.exit_code;
}

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wimplicit-fallthrough" // make gcc/clang shut up about the switch not having any breaks
#endif

void
archi_finite_state_machine_loop(
        struct archi_state_context *context)
{
    // Call the state transition function the initial time
    {
        /********************************************************/
        if (!archi_finite_state_machine_state_transition(context))
            return;
        /********************************************************/
    }

    // For all future states
    for (;;)
    {
        // Set the jump point, call the current state function and process its response after return/jump
        switch (setjmp(context->env))
        {
            case J_STATE: // no jump were performed yet, call the state function
                {
                    /***************************************/
                    context->current_state.function(context);
                    /***************************************/
                }
            case J_TRANSITION: // returned from the state function, perform transition to the next state
                {
                    /*******************************************************/
                    if (archi_finite_state_machine_state_transition(context))
                        continue;
                    /*******************************************************/
                }
            case J_EXIT: // exit the finite state machine normally
                goto final_transition;

            default: // shouldn't happen
                abort();
        }
    }

final_transition:
    // Call the state transition function the final time
    if (context->state_transition.function != NULL)
    {
        /********************************************************/
        context->state_transition.function(context->current_state,
                NULL, NULL, context->state_transition.data);
        /********************************************************/
    }
}

#ifdef __GNUC__
#  pragma GCC diagnostic pop // ignored "-Wimplicit-fallthrough"
#endif

bool
archi_finite_state_machine_state_transition(
        struct archi_state_context *context)
{
    do
    {
        if (context->state_transition.function != NULL)
        {
            // Call the state transition function
            /********************************************************/
            context->state_transition.function(context->current_state,
                    &context->next_state, &context->return_state,
                    context->state_transition.data);
            /********************************************************/
        }

        if (context->next_state.function != NULL)
        {
            // Push the return state to the stack if needed
            if (context->return_state.function != NULL)
            {
                if (!archi_state_context_stack_push(context, context->return_state))
                {
                    context->exit_code = ARCHI_ERROR_ALLOC;
                    return false; // exit caused by memory allocation fail
                }

                context->return_state = ARCHI_NULL_STATE;
            }

            break; // the next state is determined
        }
        else
        {
            // Pop the next state from the stack if needed
            if (context->return_state.function == NULL)
            {
                if (!archi_state_context_stack_pop(context, &context->next_state))
                    return false; // the stack is empty, exit now
            }
            else
            {
                context->next_state = context->return_state;
                context->return_state = ARCHI_NULL_STATE;

                break; // the next state is determined
            }
        }
    }
    while (context->next_state.function == NULL);

    // Update the current state and reset the next state
    context->current_state = context->next_state;
    context->next_state = ARCHI_NULL_STATE;

    return true; // the state transition done successfully
}

/*****************************************************************************/

archi_state_t
archi_current(
        const struct archi_state_context *context)
{
    return (context != NULL) ? context->current_state : ARCHI_NULL_STATE;
}

/*****************************************************************************/

void
archi_call(
        archi_state_t next_state,
        archi_state_t return_state,

        struct archi_state_context *context)
{
    if (context == NULL)
        return;

    // Prepare the state transition and return
    context->next_state = next_state;
    context->return_state = return_state;

    longjmp(context->env, J_TRANSITION);
}

void
archi_proceed(
        archi_state_t next_state,

        struct archi_state_context *context)
{
    archi_call(next_state, ARCHI_NULL_STATE, context);
}

void
archi_finish(
        struct archi_state_context *context)
{
    archi_proceed(ARCHI_NULL_STATE, context);
}

void
archi_exit(
        archi_status_t exit_code,

        struct archi_state_context *context)
{
    if (context == NULL)
        return;

    // Exit the finite state machine
    context->exit_code = exit_code;
    longjmp(context->env, J_EXIT);
}

/*****************************************************************************/

ARCHI_STATE_FUNCTION(archi_state_chain_execute)
{
    archi_state_chain_t *chain = ARCHI_STATE_DATA(archi_state_chain_t);
    if (chain == NULL)
        ARCHI_FINISH();

    archi_state_t return_state = {.data = chain->data};
    if (return_state.data != NULL)
        return_state.function = archi_state_chain_execute;

    ARCHI_CALL(chain->next_state, return_state);
}

