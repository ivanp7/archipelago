#include "test.h"

#include "archi/app/context.fun.h"
#include "archi/util/error.def.h"

#include <string.h>

static
ARCHI_CONTEXT_INIT_FUNC(init_func)
{
    if ((context == NULL) || (*context != NULL))
        return 1;
    else if ((config.data != ARCHI_CONTEXT_DUMMY) ||
            (config.interface != ARCHI_CONTEXT_DUMMY))
        return 2;

    *context = ARCHI_CONTEXT_DUMMY;
    return 0;
}

TEST(archi_context_initialize)
{
    archi_context_interface_t interface = {0};
    archi_context_t context = {0};
    archi_container_t config = {0};

    ASSERT_EQ(archi_context_initialize(NULL, config),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_context_initialize(&context, config),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    context.interface = &interface;
    ASSERT_EQ(archi_context_initialize(&context, config),
            0, archi_status_t, "%i");

    context.handle = NULL;

    interface.init_fn = init_func;
    ASSERT_EQ(archi_context_initialize(&context, config),
            2, archi_status_t, "%i");

    config.interface = config.data = ARCHI_CONTEXT_DUMMY;
    ASSERT_EQ(archi_context_initialize(&context, config),
            0, archi_status_t, "%i");
    ASSERT_EQ(context.handle, ARCHI_CONTEXT_DUMMY, void*, "%p");
}

static
ARCHI_CONTEXT_FINAL_FUNC(final_func)
{
    int *counter = context;
    if (counter != NULL)
        (*counter)++;
}

TEST(archi_context_finalize)
{
    int counter = 0;

    archi_context_interface_t interface = {0};
    archi_context_t context = {0};

    archi_context_finalize(&context);
    ASSERT_EQ(context.handle, NULL, void*, "%p");

    context.interface = &interface;
    archi_context_finalize(&context);
    ASSERT_EQ(context.handle, NULL, void*, "%p");

    interface.final_fn = final_func;
    archi_context_finalize(&context);
    ASSERT_EQ(context.handle, NULL, void*, "%p");

    context.handle = ARCHI_CONTEXT_DUMMY;
    archi_context_finalize(&context);
    ASSERT_EQ(context.handle, NULL, void*, "%p");

    context.handle = &counter;
    archi_context_finalize(&context);
    ASSERT_EQ(context.handle, NULL, void*, "%p");
    ASSERT_EQ(counter, 1, int, "%i");
}

static
ARCHI_CONTEXT_SET_FUNC(set_func)
{
    int *counter = context;

    if (strcmp(slot, "inc") == 0)
        (*counter) += value->num_of;
    else if (strcmp(slot, "dec") == 0)
        (*counter) -= value->num_of;
    else
        return 42;

    return 0;
}

TEST(archi_context_set)
{
    int counter = 0;

    archi_context_interface_t interface = {0};
    archi_context_t context = {0};
    archi_value_t value = {0};

    ASSERT_EQ(archi_context_set(context, NULL, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    context.handle = &counter;
    ASSERT_EQ(archi_context_set(context, NULL, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    context.interface = &interface;
    ASSERT_EQ(archi_context_set(context, NULL, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    ASSERT_EQ(archi_context_set(context, "inc", NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_context_set(context, NULL, &value),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    ASSERT_EQ(archi_context_set(context, "inc", &value),
            ARCHI_ERROR_FUNCTION, archi_status_t, "%i");

    interface.set_fn = set_func;
    ASSERT_EQ(archi_context_set(context, "inc", &value),
            0, archi_status_t, "%i");
    ASSERT_EQ(counter, 0, int, "%i");

    value.num_of = 13;
    ASSERT_EQ(archi_context_set(context, "inc", &value),
            0, archi_status_t, "%i");
    ASSERT_EQ(counter, 13, int, "%i");

    value.num_of = 6;
    ASSERT_EQ(archi_context_set(context, "dec", &value),
            0, archi_status_t, "%i");
    ASSERT_EQ(counter, 7, int, "%i");

    ASSERT_EQ(archi_context_set(context, "inv", &value),
            42, archi_status_t, "%i");
}

static
ARCHI_CONTEXT_GET_FUNC(get_func)
{
    int *counter = context;

    if (strcmp(slot, "inc") == 0)
        value->num_of += *counter;
    else if (strcmp(slot, "dec") == 0)
        value->num_of -= *counter;
    else
        return 42;

    return 0;
}

TEST(archi_context_get)
{
    int counter = 0;

    archi_context_interface_t interface = {0};
    archi_context_t context = {0};
    archi_value_t value = {0};

    ASSERT_EQ(archi_context_get(context, NULL, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    context.handle = &counter;
    ASSERT_EQ(archi_context_get(context, NULL, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    context.interface = &interface;
    ASSERT_EQ(archi_context_get(context, NULL, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    ASSERT_EQ(archi_context_get(context, "inc", NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_context_get(context, NULL, &value),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    ASSERT_EQ(archi_context_get(context, "inc", &value),
            ARCHI_ERROR_FUNCTION, archi_status_t, "%i");

    interface.get_fn = get_func;

    counter = 3;
    ASSERT_EQ(archi_context_get(context, "inc", &value),
            0, archi_status_t, "%i");
    ASSERT_EQ(value.num_of, 3, int, "%i");

    counter = 5;
    ASSERT_EQ(archi_context_get(context, "inc", &value),
            0, archi_status_t, "%i");
    ASSERT_EQ(value.num_of, 8, int, "%i");

    ASSERT_EQ(archi_context_get(context, "dec", &value),
            0, archi_status_t, "%i");
    ASSERT_EQ(value.num_of, 3, int, "%i");

    ASSERT_EQ(archi_context_get(context, "inv", &value),
            42, archi_status_t, "%i");
}

TEST(archi_context_assign)
{
    int counter1 = 0, counter2 = 0;

    archi_context_interface_t interface = {0};
    archi_context_t context1 = {0}, context2 = {0};

    ASSERT_EQ(archi_context_assign(context1, NULL, context2, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    context1.handle = &counter1;
    context2.handle = &counter2;
    ASSERT_EQ(archi_context_assign(context1, NULL, context2, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    context1.interface = context2.interface = &interface;
    ASSERT_EQ(archi_context_assign(context1, NULL, context2, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    ASSERT_EQ(archi_context_assign(context1, "inc", context2, "dec"),
            ARCHI_ERROR_FUNCTION, archi_status_t, "%i");

    counter2 = 13;

    interface.set_fn = set_func;
    interface.get_fn = get_func;
    ASSERT_EQ(archi_context_assign(context1, "inc", context2, "dec"),
            0, archi_status_t, "%i");
    ASSERT_EQ(counter1, -13, int, "%i");

    ASSERT_EQ(archi_context_assign(context1, "inc", context2, "inc"),
            0, archi_status_t, "%i");
    ASSERT_EQ(counter1, 0, int, "%i");

    ASSERT_EQ(archi_context_assign(context1, "dec", context2, "inc"),
            0, archi_status_t, "%i");
    ASSERT_EQ(counter1, -13, int, "%i");

    ASSERT_EQ(archi_context_assign(context1, "inv", context2, "inc"),
            42, archi_status_t, "%i");
    ASSERT_EQ(counter1, -13, int, "%i");
}

static
ARCHI_CONTEXT_ACT_FUNC(act_func)
{
    (void) params;

    int *counter = context;

    if (strcmp(action, "inc") == 0)
        (*counter)++;
    else if (strcmp(action, "dec") == 0)
        (*counter)--;
    else
        return 42;

    return 0;
}

TEST(archi_context_act)
{
    int counter = 0;

    archi_context_interface_t interface = {0};
    archi_context_t context = {0};
    archi_container_t params = {0};

    ASSERT_EQ(archi_context_act(context, NULL, params),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_context_act(context, "inc", params),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    context.handle = &counter;
    ASSERT_EQ(archi_context_act(context, "inc", params),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    context.interface = &interface;
    ASSERT_EQ(archi_context_act(context, "inc", params),
            ARCHI_ERROR_FUNCTION, archi_status_t, "%i");

    interface.act_fn = act_func;
    ASSERT_EQ(archi_context_act(context, "inc", params),
            0, archi_status_t, "%i");
    ASSERT_EQ(counter, 1, int, "%i");

    ASSERT_EQ(archi_context_act(context, "dec", params),
            0, archi_status_t, "%i");
    ASSERT_EQ(counter, 0, int, "%i");

    ASSERT_EQ(archi_context_act(context, "inv", params),
            42, archi_status_t, "%i");
}

