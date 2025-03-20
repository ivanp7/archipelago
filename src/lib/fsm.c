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
 * @brief Finite state machines implementation.
 */

#include "archi/fsm/algorithm.fun.h"
#include "archi/fsm/state.fun.h"
#include "archi/util/error.def.h"

#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>

#define ARCHI_FSM_INITIAL_STACK_CAPACITY 32

enum archi_fsm_mode {
    J_STATE = 0,
    J_TRANSITION,
};

struct archi_fsm_context {
    archi_fsm_state_t current_state;
    archi_fsm_transition_t transition;

    archi_fsm_state_t *stack;
    size_t stack_size;
    size_t stack_capacity;

    archi_status_t code;

    enum archi_fsm_mode mode;
    jmp_buf env; // non-local jumps
};

static
bool
archi_fsm_state_context_stack_reserve(
        struct archi_fsm_context *context,
        size_t size)
{
    if (context->stack_capacity >= context->stack_size + size)
        return true; // the current capacity is enough

    size_t new_stack_capacity = context->stack_capacity;
    do
        new_stack_capacity *= 2; // double the stack capacity
    while (new_stack_capacity < context->stack_size + size);

    archi_fsm_state_t *new_stack = realloc(context->stack,
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
archi_fsm_loop(
        struct archi_fsm_context *const context);

static
bool
archi_fsm_transition(
        struct archi_fsm_context *const context);

archi_status_t
archi_fsm_execute(
        archi_fsm_t fsm)
{
    // Process the degenerate case
    if ((fsm.entry_state.function == NULL) && (fsm.transition.function == NULL))
        return 0;

    // Initialize the context
    struct archi_fsm_context context = {
        .transition = fsm.transition,

        .stack_size = 1,
        .stack_capacity = ARCHI_FSM_INITIAL_STACK_CAPACITY,

        .mode = J_TRANSITION,
    };

    // Allocate the stack memory
    context.stack = malloc(sizeof(*context.stack) * context.stack_capacity);
    if (context.stack == NULL)
        return ARCHI_ERROR_ALLOC;

    context.stack[0] = fsm.entry_state;

    // Run the finite state machine loop
    /***********************/
    archi_fsm_loop(&context);
    /***********************/

    // Free the stack memory and return
    free(context.stack);

    return context.code;
}

void
archi_fsm_loop(
        struct archi_fsm_context *const context)
{
    // This function is needed to get rid of as many local variables as possible.
    // The only local variable left is `context` pointer.
    // As it does not change its address, it is not required to be volatile (setjmp() restriction).

    while (archi_fsm_transition(context)) // there is a next state
    {
        // Set the jump point, call the current state function and process its response after return/jump
        switch (setjmp(context->env))
        {
            case J_STATE: // no jump were performed yet, call the state function
                context->mode = J_STATE;
                {
                    /***************************************/
                    context->current_state.function(context);
                    /***************************************/
                }
                // fallthrough
            case J_TRANSITION: // returned from the state function, perform transition to the next state
                context->mode = J_TRANSITION;
                continue;

            default: // shouldn't happen
                abort();
        }
    }
}

bool
archi_fsm_transition(
        struct archi_fsm_context *const context)
{
    if (context->transition.function == NULL)
    {
        if (context->stack_size > 0) // pop the next state from the stack
            context->current_state = context->stack[--context->stack_size];
        else // the stack is empty, exit now
            return false;
    }
    else
    {
        archi_fsm_state_t stack_top, trans_state = {0};

        if (context->stack_size > 0) // the next state is at the stack top
            stack_top = context->stack[context->stack_size - 1];
        else
            stack_top = ARCHI_NULL_FSM_STATE;

        // Call the state transition function
        {
            /**************************************************************/
            context->transition.function(context->current_state, stack_top,
                    &trans_state, &context->code, context->transition.data);
            /**************************************************************/
        }

        // Update the current state
        if (trans_state.function != NULL)
            context->current_state = trans_state;
        else if (stack_top.function != NULL)
        {
            context->current_state = stack_top;
            context->stack_size--; // pop the top from the stack
        }
        else // the stack is empty, exit now
            return false;
    }

    return true;
}

/*****************************************************************************/

archi_fsm_state_t
archi_fsm_current(
        const struct archi_fsm_context *context)
{
    return (context != NULL) ? context->current_state : ARCHI_NULL_FSM_STATE;
}

size_t
archi_fsm_stack_size(
        const struct archi_fsm_context *context)
{
    return (context != NULL) ? context->stack_size : 0;
}

archi_status_t
archi_fsm_code(
        const struct archi_fsm_context *context)
{
    return (context != NULL) ? context->code : 0;
}

void
archi_fsm_set_code(
        struct archi_fsm_context *context,

        archi_status_t code)
{
    if ((context == NULL) || (context->mode != J_STATE))
        return;

    context->code = code;
}

/*****************************************************************************/

static
void
archi_fsm_return(
        struct archi_fsm_context *context)
{
    longjmp(context->env, J_TRANSITION);
}

static
void
archi_fsm_error(
        struct archi_fsm_context *context,
        archi_status_t code)
{
    archi_fsm_set_code(context, code);
    context->stack_size = 0;

    archi_fsm_return(context);
}

void
archi_fsm_proceed(
        struct archi_fsm_context *context,

        size_t num_popped,
        size_t num_pushed,
        const archi_fsm_state_t pushed[])
{
    if ((context == NULL) || (context->mode != J_STATE))
        return;

    if (num_popped > context->stack_size)
        archi_fsm_error(context, ARCHI_ERROR_MISUSE);
    else if ((num_pushed > 0) && (pushed == NULL))
        archi_fsm_error(context, ARCHI_ERROR_MISUSE);

    // Pop states from the stack
    context->stack_size -= num_popped;

    // Push states to the stack in reverse order
    for (size_t i = num_pushed; i-- > 0;)
    {
        archi_fsm_state_t state = pushed[i];

        if (state.function == NULL)
            continue; // don't push null states to the stack

        // Reserve a seat in the stack
        if (!archi_fsm_state_context_stack_reserve(context, 1))
            archi_fsm_error(context, ARCHI_ERROR_ALLOC);

        // Push
        context->stack[context->stack_size++] = state;
    }

    // Proceed
    archi_fsm_return(context);
}

/*****************************************************************************/

ARCHI_FSM_STATE_FUNCTION(archi_fsm_state_chain_execute)
{
    archi_fsm_state_chain_t *chain = ARCHI_FSM_CURRENT_DATA(archi_fsm_state_chain_t);
    if (chain == NULL)
        ARCHI_FSM_DONE(0);

    archi_fsm_state_t next_link = {.data = chain->data};
    if (next_link.data != NULL)
        next_link.function = archi_fsm_state_chain_execute;

    ARCHI_FSM_PROCEED(0, chain->next_state, next_link);
}

