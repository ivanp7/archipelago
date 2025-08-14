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
 * @brief Hierarchical state processor implementation.
 */

#include "archi/hsp/api/state.fun.h"
#include "archi/hsp/api/exec.fun.h"
#include "archipelago/util/size.def.h"

#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>

#define ARCHI_HSP_INITIAL_STACK_CAPACITY 32

/*****************************************************************************/

archi_hsp_frame_t*
archi_hsp_frame_alloc(
        size_t num_states)
{
    archi_hsp_frame_t *frame = malloc(
            ARCHI_SIZEOF_FLEXIBLE(archi_hsp_frame_t, state, num_states));
    if (frame == NULL)
        return NULL;

    size_t *num_states_ptr = (size_t*)&frame->num_states;
    *num_states_ptr = num_states;

    for (size_t i = 0; i < num_states; i++)
        frame->state[i] = (archi_hsp_state_t){0};

    return frame;
}

/*****************************************************************************/

enum archi_hsp_mode {
    J_STATE = 0,  // zero: setjmp() returns without jump
    J_TRANSITION, // non-zero: setjmp() returns after jump
};

struct archi_hsp_context {
    archi_hsp_state_t current_state;
    archi_hsp_transition_t transition;

    archi_hsp_state_t *stack;
    size_t *stack_frames;

    size_t stack_capacity;
    size_t stack_size;
    size_t num_stack_frames;

    archi_status_t code;

    enum archi_hsp_mode mode;
    jmp_buf env; // non-local jumps
};

/*****************************************************************************/

archi_hsp_state_t
archi_hsp_current_state(
        const struct archi_hsp_context *context)
{
    return (context != NULL) ? context->current_state : (archi_hsp_state_t){0};
}

size_t
archi_hsp_stack_frames(
        const struct archi_hsp_context *context)
{
    return (context != NULL) ? context->num_stack_frames : 0;
}

/*****************************************************************************/

static
void
archi_hsp_state_context_stack_reserve(
        struct archi_hsp_context *context,
        size_t size)
{
    if (context->stack_capacity >= context->stack_size + size)
        return; // the current capacity is enough

    size_t new_stack_capacity = context->stack_capacity;
    do
        new_stack_capacity *= 2; // double the stack capacity
    while (new_stack_capacity < context->stack_size + size);

    context->stack_capacity = new_stack_capacity;

    archi_hsp_state_t *new_stack = realloc(context->stack,
            sizeof(*new_stack) * new_stack_capacity);
    if (new_stack == NULL)
        archi_hsp_abort(context, ARCHI_STATUS_ENOMEMORY);

    context->stack = new_stack;

    size_t *new_stack_frames = realloc(context->stack_frames,
            sizeof(*new_stack_frames) * new_stack_capacity);
    if (new_stack_frames == NULL)
        archi_hsp_abort(context, ARCHI_STATUS_ENOMEMORY);

    context->stack_frames = new_stack_frames;
}

static
void
archi_hsp_advance_impl(
        struct archi_hsp_context *context,

        size_t num_popped_frames,

        size_t num_pushed_states,
        const archi_hsp_state_t *pushed_states)
{
    bool new_frame_needed = true;

    // Pop states from the stack
    if (num_popped_frames > 0)
        context->stack_size = context->stack_frames[context->num_stack_frames -= num_popped_frames];
    else
    {
        if ((context->num_stack_frames > 0) &&
                (context->stack_size == context->stack_frames[context->num_stack_frames - 1]))
            new_frame_needed = false; // the current frame is empty and can be reused
    }

    size_t current_frame = context->stack_size;

    // Count non-NULL states and reserve seats in the stack for them
    size_t seats_required = 0;
    for (size_t i = 0; i < num_pushed_states; i++)
        if (pushed_states[i].function != NULL)
            seats_required++;

    archi_hsp_state_context_stack_reserve(context, seats_required);

    // Push states to the stack in reverse order
    for (size_t i = num_pushed_states; i-- > 0;)
        if (pushed_states[i].function != NULL)
            context->stack[context->stack_size++] = pushed_states[i];

    // Add the new frame if needed
    if ((context->stack_size > current_frame) && new_frame_needed)
        context->stack_frames[context->num_stack_frames++] = current_frame;
}

static
void
archi_hsp_longjump(
        struct archi_hsp_context *context)
{
    longjmp(context->env, J_TRANSITION);
}

void
archi_hsp_advance(
        struct archi_hsp_context *context,

        size_t num_popped_frames,

        size_t num_pushed_states,
        const archi_hsp_state_t *pushed_states)
{
    if ((context == NULL) || (context->mode != J_STATE))
        return;

    if (num_popped_frames > context->num_stack_frames)
        archi_hsp_abort(context, ARCHI_STATUS_EMISUSE);
    else if ((num_pushed_states > 0) && (pushed_states == NULL))
        archi_hsp_abort(context, ARCHI_STATUS_EMISUSE);

    archi_hsp_advance_impl(context, num_popped_frames, num_pushed_states, pushed_states);

    // Proceed and don't return
    archi_hsp_longjump(context);
}

void
archi_hsp_abort(
        struct archi_hsp_context *context,
        archi_status_t code)
{
    if ((context == NULL) || (context->mode != J_STATE) || (code == 0))
        return;

    context->code = code;
    archi_hsp_longjump(context);
}

/*****************************************************************************/

static
bool
archi_hsp_transition(
        struct archi_hsp_context *const context)
{
    archi_hsp_state_t next_state = {0}, trans_state = {0};

    if (context->stack_size > 0)
        next_state = context->stack[context->stack_size - 1]; // stack top

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

static
void
archi_hsp_loop(
        struct archi_hsp_context *const context)
{
    // This function is needed to get rid of local variables in the stack frame,
    // as setjmp() requires modifiable local variables to be volatile.
    // The only local variable left is `context` pointer,
    // but its address is constant and thus not required to be volatile.

    while ((context->code == 0) && archi_hsp_transition(context)) // no errors and there is a next state
    {
        // Set the jump point and call the current state function
        switch (setjmp(context->env))
        {
            case J_STATE: // hasn't jumped yet, call the state function
                context->mode = J_STATE;
                {
                    /***************************************/
                    context->current_state.function(context);
                    /***************************************/
                }
                // fallthrough
            case J_TRANSITION: // returned from the state function
                context->mode = J_TRANSITION;
                continue;

            default: // shouldn't happen
                abort();
        }
    }
}

archi_status_t
archi_hsp_execute(
        const archi_hsp_frame_t *entry_frame,
        archi_hsp_transition_t transition)
{
    if ((entry_frame == NULL) || (entry_frame->num_states == 0))
        return 0;

    // Initialize the context
    struct archi_hsp_context context = {
        .transition = transition,

        .stack_capacity = ARCHI_HSP_INITIAL_STACK_CAPACITY,
        .stack_size = 1,
        .num_stack_frames = 1,

        .mode = J_TRANSITION,
    };

    // Allocate the stack memory
    context.stack = malloc(sizeof(*context.stack) * context.stack_capacity);
    if (context.stack == NULL)
        return ARCHI_STATUS_ENOMEMORY;

    context.stack_frames = malloc(sizeof(*context.stack_frames) * context.stack_capacity);
    if (context.stack_frames == NULL)
    {
        free(context.stack);
        return ARCHI_STATUS_ENOMEMORY;
    }

    // Push the initial frame to the stack
    archi_hsp_advance_impl(&context, 0, entry_frame->num_states, entry_frame->state);

    // Run the hierarchical state processor loop
    archi_hsp_loop(&context);

    // Free the stack memory and return
    free(context.stack);
    free(context.stack_frames);

    return context.code;
}

