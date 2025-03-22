#include "test.h"

#include "archi/fsm/algorithm.fun.h"
#include "archi/fsm/state.fun.h"

#define NUM 100

static
ARCHI_FSM_STATE_FUNCTION(state_dec)
{
    int *state_counter = ARCHI_FSM_CURRENT().data;
    (*state_counter)--;
}

static
ARCHI_FSM_STATE_FUNCTION(state_inc)
{
    int *state_counter = ARCHI_FSM_CURRENT().data;
    (*state_counter)++;

    if (*state_counter < NUM)
        ARCHI_FSM_PROCEED(0, ARCHI_FSM_CURRENT(), ARCHI_FSM_CURRENT());
    else
        ARCHI_FSM_FINISH(NUM - 1);
}

static
ARCHI_FSM_TRANSITION_FUNCTION(transition0)
{
    int *trans_counter = data;
    (*trans_counter)++;

    if ((prev_state.function == state_inc) && (next_state.function == NULL))
        *trans_state = ARCHI_FSM_STATE_OTHER_FUNC(prev_state, state_dec);
}

TEST(archi_fsm_execute)
{
    int state_counter = 0;
    int trans_counter = 0;

    archi_status_t code;

    code = archi_fsm_execute((archi_fsm_t){.entry_state = ARCHI_FSM_STATE(state_inc, &state_counter),
            .transition = ARCHI_FSM_TRANSITION(transition0, &trans_counter)});

    ASSERT_EQ(code, 0, archi_status_t, "%i");
    ASSERT_EQ(state_counter, NUM - 1, int, "%i");
    ASSERT_EQ(trans_counter, 1 + NUM + 1, int, "%i");
}

TEST(archi_fsm_select_unconditionally)
{
    ASSERT_EQ(archi_fsm_select_unconditionally(NULL), 0, size_t, "%lu");

    for (size_t index = 0; index < 8; index++)
        ASSERT_EQ(archi_fsm_select_unconditionally(&index), index, size_t, "%lu");
}

static
ARCHI_FSM_STATE_FUNCTION(state1)
{
    int *counter1 = ARCHI_FSM_CURRENT().data;
    (*counter1)++;
}

static
ARCHI_FSM_STATE_FUNCTION(state2)
{
    int *counter2 = ARCHI_FSM_CURRENT().data;
    (*counter2)--;
}

TEST(archi_fsm_state_proceed)
{
    int counter[2] = {0};

    archi_fsm_state_t states[] = {
        {.function = state1, .data = &counter[0]},
        {.function = state1, .data = &counter[0]},
        {.function = state2, .data = &counter[1]},
        {.function = state2, .data = &counter[1]},

        {.function = state2, .data = &counter[0]},
        {.function = state1, .data = &counter[1]},
    };

    archi_fsm_stack_frame_t frame = {.states = states,
        .length = sizeof(states) / sizeof(states[0])};

    archi_status_t code;

    code = archi_fsm_execute((archi_fsm_t){
            .entry_state = ARCHI_FSM_STATE(archi_fsm_state_proceed, &frame)});

    ASSERT_EQ(code, 0, archi_status_t, "%i");
    ASSERT_EQ(counter[0], +1, int, "%i");
    ASSERT_EQ(counter[1], -1, int, "%i");
}

TEST(archi_fsm_state_branch)
{
    int counter[2] = {0};

    archi_fsm_state_t states[2] = {
        {.function = state1, .data = &counter[0]},
        {.function = state2, .data = &counter[1]},
    };

    archi_fsm_stack_frame_t frame[2] = {
        {.states = &states[0], .length = 1},
        {.states = &states[1], .length = 1},
    };

    archi_fsm_state_branch_data_t branch = {
        .selector_fn = archi_fsm_select_unconditionally,
        .frames = frame,
    };

    archi_status_t code;

    code = archi_fsm_execute((archi_fsm_t){
            .entry_state = ARCHI_FSM_STATE(archi_fsm_state_branch, &branch)});

    ASSERT_EQ(code, 0, archi_status_t, "%i");
    ASSERT_EQ(counter[0], +1, int, "%i");
    ASSERT_EQ(counter[1], 0, int, "%i");

    size_t index = 0;
    branch.selector_data = &index;

    code = archi_fsm_execute((archi_fsm_t){
            .entry_state = ARCHI_FSM_STATE(archi_fsm_state_branch, &branch)});

    ASSERT_EQ(code, 0, archi_status_t, "%i");
    ASSERT_EQ(counter[0], +2, int, "%i");
    ASSERT_EQ(counter[1], 0, int, "%i");

    index++;

    code = archi_fsm_execute((archi_fsm_t){
            .entry_state = ARCHI_FSM_STATE(archi_fsm_state_branch, &branch)});

    ASSERT_EQ(code, 0, archi_status_t, "%i");
    ASSERT_EQ(counter[0], +2, int, "%i");
    ASSERT_EQ(counter[1], -1, int, "%i");
}

