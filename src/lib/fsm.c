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
    size_t *stack_frames;

    size_t stack_capacity;
    size_t stack_size;
    size_t num_stack_frames;

    archi_status_t code;

    enum archi_fsm_mode mode;
    jmp_buf env; // non-local jumps
};

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

        .stack_capacity = ARCHI_FSM_INITIAL_STACK_CAPACITY,
        .stack_size = 1,
        .num_stack_frames = 1,

        .mode = J_TRANSITION,
    };

    // Allocate the stack memory
    context.stack = malloc(sizeof(*context.stack) * context.stack_capacity);
    if (context.stack == NULL)
        return ARCHI_ERROR_ALLOC;

    context.stack_frames = malloc(sizeof(*context.stack_frames) * context.stack_capacity);
    if (context.stack_frames == NULL)
    {
        free(context.stack);
        return ARCHI_ERROR_ALLOC;
    }

    context.stack[0] = fsm.entry_state;
    context.stack_frames[0] = 0;

    // Run the finite state machine loop
    archi_fsm_loop(&context);

    // Free the stack memory and return
    free(context.stack);
    free(context.stack_frames);

    return context.code;
}

void
archi_fsm_loop(
        struct archi_fsm_context *const context)
{
    // This function is needed to get rid of local variables in the stack frame,
    // as setjmp() requires modifiable local variables to be volatile.
    // The only local variable left is `context` pointer,
    // but its address is constant and thus not required to be volatile.

    while ((context->code == 0) && archi_fsm_transition(context)) // no errors and there is a next state
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
    archi_fsm_state_t next_state, trans_state = {0};

    if (context->stack_size > 0)
        next_state = context->stack[context->stack_size - 1]; // stack top
    else
        next_state = ARCHI_NULL_FSM_STATE;

    // Call the state transition function
    if (context->transition.function != NULL)
    {
        /**********************************************************/
        context->transition.function(context->current_state,
                next_state, &trans_state, context->transition.data);
        /**********************************************************/
    }

    // Update the current state
    if (trans_state.function != NULL)
        context->current_state = trans_state;
    else if (next_state.function != NULL)
    {
        context->current_state = next_state;
        context->stack_size--;

        // Delete the finished frame
        if ((context->num_stack_frames > 0) &&
                (context->stack_size < context->stack_frames[context->num_stack_frames - 1]))
            context->num_stack_frames--;
    }
    else // the stack is empty, exit now
        return false;

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
archi_fsm_stack_frames(
        const struct archi_fsm_context *context)
{
    return (context != NULL) ? context->num_stack_frames : 0;
}

/*****************************************************************************/

static
void
archi_fsm_longjump(
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
    context->code = code;
    archi_fsm_longjump(context);
}

static
void
archi_fsm_state_context_stack_reserve(
        struct archi_fsm_context *context,
        size_t size)
{
    if (context->stack_capacity >= context->stack_size + size)
        return; // the current capacity is enough

    size_t new_stack_capacity = context->stack_capacity;
    do
        new_stack_capacity *= 2; // double the stack capacity
    while (new_stack_capacity < context->stack_size + size);

    archi_fsm_state_t *new_stack = realloc(context->stack,
            sizeof(*context->stack) * new_stack_capacity);
    if (new_stack == NULL)
        archi_fsm_error(context, ARCHI_ERROR_ALLOC);

    size_t *new_stack_frames = realloc(context->stack_frames,
            sizeof(*context->stack_frames) * new_stack_capacity);
    if (new_stack_frames == NULL)
        archi_fsm_error(context, ARCHI_ERROR_ALLOC);

    context->stack_capacity = new_stack_capacity;
    context->stack = new_stack;
    context->stack_frames = new_stack_frames;
}

void
archi_fsm_proceed(
        struct archi_fsm_context *context,

        size_t pop_frames,

        const archi_fsm_state_t frame[],
        size_t frame_length)
{
    if ((context == NULL) || (context->mode != J_STATE))
        return;

    if (pop_frames > context->num_stack_frames)
        archi_fsm_error(context, ARCHI_ERROR_MISUSE);
    else if ((frame_length > 0) && (frame == NULL))
        archi_fsm_error(context, ARCHI_ERROR_MISUSE);

    bool new_frame_needed = true;

    // Pop states from the stack
    if (pop_frames > 0)
        context->stack_size = context->stack_frames[context->num_stack_frames -= pop_frames];
    else
    {
        if ((context->num_stack_frames > 0) &&
                (context->stack_size == context->stack_frames[context->num_stack_frames - 1]))
            new_frame_needed = false; // the current frame is empty and can be reused
    }

    // Push states to the stack in reverse order
    size_t current_frame = context->stack_size;

    for (size_t i = frame_length; i-- > 0;)
    {
        archi_fsm_state_t state = frame[i];

        if (state.function == NULL)
            continue; // don't push null states to the stack

        // Ensure there is a seat in the stack
        archi_fsm_state_context_stack_reserve(context, 1);

        // Push
        context->stack[context->stack_size++] = state;
    }

    // Add the new frame if needed
    if ((context->stack_size > current_frame) && new_frame_needed)
        context->stack_frames[context->num_stack_frames++] = current_frame;

    // Proceed and don't return
    archi_fsm_longjump(context);
}

/*****************************************************************************/

ARCHI_FSM_STATE_FUNCTION(archi_fsm_state_chain_execute)
{
    archi_fsm_state_chain_t *chain = ARCHI_FSM_CURRENT_DATA(archi_fsm_state_chain_t*);

    if (chain == NULL)
        return;

    archi_fsm_state_t next_link = {.data = chain->data};
    if (next_link.data != NULL)
        next_link.function = archi_fsm_state_chain_execute;

    ARCHI_FSM_PROCEED(0, chain->next_state, next_link);
}

