#include "test.h"

#include "archi/fsm/algorithm.fun.h"
#include "archi/fsm/state.fun.h"

#define NUM 100

static
ARCHI_FSM_STATE_FUNCTION(state_dec)
{
    int *state_counter = ARCHI_FSM_CURRENT_DATA(int*);
    (*state_counter)--;
}

static
ARCHI_FSM_STATE_FUNCTION(state_inc)
{
    int *state_counter = ARCHI_FSM_CURRENT_DATA(int*);
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
    archi_status_t code;

    int state_counter = 0;
    int trans_counter = 0;

    code = archi_fsm_execute((archi_fsm_t){.entry_state = ARCHI_FSM_STATE(state_inc, &state_counter),
            .transition = ARCHI_FSM_TRANSITION(transition0, &trans_counter)});

    ASSERT_EQ(code, 0, archi_status_t, "%i");
    ASSERT_EQ(state_counter, NUM - 1, int, "%i");
    ASSERT_EQ(trans_counter, 1 + NUM + 1, int, "%i");
}

static
ARCHI_FSM_STATE_FUNCTION(state1)
{
    int *counter1 = ARCHI_FSM_CURRENT_DATA(int*);

    (*counter1)++;

    if (*counter1 == NUM)
        ARCHI_FSM_FINISH(ARCHI_FSM_STACK_FRAMES());
}

static
ARCHI_FSM_STATE_FUNCTION(state2)
{
    int *counter2 = ARCHI_FSM_CURRENT_METADATA(int*);

    (*counter2)++;
}

TEST(archi_fsm_state_chain_execute)
{
    archi_status_t code;

    int counter1 = 0, counter2 = 0;

    archi_fsm_state_chain_t chain1 = {.next_state = ARCHI_FSM_STATE_M(state2, NULL, &counter2)};
    archi_fsm_state_chain_t chain2 = {.next_state = ARCHI_FSM_STATE(state1, &counter1)};

    chain1.data = &chain2;
    chain2.data = &chain1;

    code = archi_fsm_execute((archi_fsm_t){.entry_state = ARCHI_FSM_STATE(archi_fsm_state_chain_execute, &chain2),
            .transition = ARCHI_NULL_FSM_TRANSITION});

    ASSERT_EQ(code, 0, archi_status_t, "%i");
    ASSERT_EQ(counter1, NUM, int, "%i");
    ASSERT_EQ(counter2, NUM - 1, int, "%i");
}

