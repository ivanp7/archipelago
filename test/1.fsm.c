#include "test.h"

#include "archi/fsm/algorithm.h"
#include "archi/fsm/state.fun.h"

#define NUM 100

static
ARCHI_STATE_FUNCTION(state_dec)
{
    ARCHI_SET_CODE(ARCHI_CODE() - 1);

    if (ARCHI_CODE() > 0)
        ARCHI_PROCEED(0, ARCHI_CURRENT());
}

static
ARCHI_STATE_FUNCTION(state_inc)
{
    int *state_counter = ARCHI_CURRENT_DATA(int);
    (*state_counter)++;

    ARCHI_SET_CODE(ARCHI_CODE() + 1);

    if (*state_counter < NUM)
        ARCHI_PROCEED(0, ARCHI_CURRENT(), ARCHI_CURRENT());
    else
        ARCHI_DONE(ARCHI_STACK_SIZE());
}

static
ARCHI_TRANSITION_FUNCTION(transition0)
{
    (void) code;

    int *trans_counter = data;
    (*trans_counter)++;

    if ((prev_state.function == state_inc) && (next_state.function == NULL))
        *trans_state = ARCHI_STATE_OTHER_FUNC(prev_state, state_dec);
}

TEST(archi_finite_state_machine)
{
    archi_status_t code;

    int state_counter = 0;
    int trans_counter = 0;

    code = archi_finite_state_machine(ARCHI_STATE(state_inc, &state_counter),
            ARCHI_TRANSITION(transition0, &trans_counter));

    ASSERT_EQ(code, 0, archi_status_t, "%i");
    ASSERT_EQ(state_counter, NUM, int, "%i");
    ASSERT_EQ(trans_counter, 2*NUM + 1, int, "%i");
}

static
ARCHI_STATE_FUNCTION(state1)
{
    int *counter1 = ARCHI_CURRENT_DATA(int);

    (*counter1)++;

    if (*counter1 == NUM)
        ARCHI_DONE(ARCHI_STACK_SIZE());
}

static
ARCHI_STATE_FUNCTION(state2)
{
    int *counter2 = ARCHI_CURRENT_METADATA(int);

    (*counter2)++;
}

TEST(archi_state_chain_execute)
{
    archi_status_t code;

    int counter1 = 0, counter2 = 0;

    archi_state_chain_t chain1 = {.next_state = ARCHI_STATE_M(state2, NULL, &counter2)};
    archi_state_chain_t chain2 = {.next_state = ARCHI_STATE(state1, &counter1)};

    chain1.data = &chain2;
    chain2.data = &chain1;

    code = archi_finite_state_machine(ARCHI_STATE(archi_state_chain_execute, &chain2),
            ARCHI_NULL_TRANSITION);

    ASSERT_EQ(code, 0, archi_status_t, "%i");
    ASSERT_EQ(counter1, NUM, int, "%i");
    ASSERT_EQ(counter2, NUM - 1, int, "%i");
}

