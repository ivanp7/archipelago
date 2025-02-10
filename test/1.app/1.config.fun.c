#include "test.h"

#include "archi/app/config.fun.h"
#include "archi/app/config.typ.h"
#include "archi/app/context.typ.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <string.h>

#define INTERFACE "iface"

struct context {
    int counter;

    int init_counter;
    int final_counter;
    int set_counter;
    int get_counter;
    int act_counter;
};

static
struct context global_context;

static
ARCHI_CONTEXT_INIT_FUNC(init_func)
{
    (void) config;
    *context = &global_context;
    global_context.init_counter++;
    return 0;
}

static
ARCHI_CONTEXT_FINAL_FUNC(final_func)
{
    struct context *context_ptr = context;
    context_ptr->final_counter++;
}

static
ARCHI_CONTEXT_SET_FUNC(set_func)
{
    struct context *context_ptr = context;

    if (strcmp(slot, "plus") == 0)
    {
        context_ptr->set_counter++;
        context_ptr->counter += value->num_of;
        return 0;
    }
    else if (strcmp(slot, "minus") == 0)
    {
        context_ptr->set_counter++;
        context_ptr->counter -= value->num_of;
        return 0;
    }
    else
        return 1;
}

static
ARCHI_CONTEXT_GET_FUNC(get_func)
{
    struct context *context_ptr = context;

    if (strcmp(slot, "value") == 0)
    {
        context_ptr->get_counter++;
        value->num_of = context_ptr->counter;
        return 0;
    }
    else
        return 1;
}

static
ARCHI_CONTEXT_ACT_FUNC(act_func)
{
    (void) params;

    struct context *context_ptr = context;

    if (strcmp(action, "leet") == 0)
    {
        context_ptr->act_counter++;
        context_ptr->counter = 1337;
        return 0;
    }
    else
        return 1;
}

TEST(archi_app_apply_config_step)
{
    archi_context_interface_t context_interface = {
        .init_fn = init_func,
        .final_fn = final_func,
        .set_fn = set_func,
        .get_fn = get_func,
        .act_fn = act_func,
    };
    archi_list_node_named_value_t context_interface_node = {
        .base.name = INTERFACE,
        .value = {.ptr = &context_interface, .num_of = 1, .type = ARCHI_VALUE_DATA},
    };

    archi_list_container_data_t contexts_data = {.insert_to_head = false, .traverse_from_head = false};
    archi_list_container_data_t context_interfaces_data = {
        .list = {(archi_list_node_t*)&context_interface_node, (archi_list_node_t*)&context_interface_node},
        .traverse_from_head = true,
    };

    archi_app_apply_config_step_data_t step_data = {
        .contexts = {.data = &contexts_data, .interface = &archi_list_container_interface},
        .context_interfaces = {.data = &context_interfaces_data, .interface = &archi_list_container_interface},
    };
    archi_app_config_step_t step;

    step.type = ARCHI_APP_CONFIG_STEP_INIT;
    step.as_init = (archi_app_config_step_init_t){
        .key = "1",
        .interface_key = INTERFACE,
    };

    ASSERT_EQ(archi_app_apply_config_step(NULL, &step, &step_data), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context.init_counter, 1, int, "%i");

    step.as_init.key = "2";
    ASSERT_EQ(archi_app_apply_config_step(NULL, &step, &step_data), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context.init_counter, 2, int, "%i");

    ASSERT_NE(contexts_data.list.head, NULL, void*, "%p");
    ASSERT_NE(contexts_data.list.tail, NULL, void*, "%p");
    ASSERT_EQ(contexts_data.list.head->next, contexts_data.list.tail, void*, "%p");
    ASSERT_EQ(contexts_data.list.tail->prev, contexts_data.list.head, void*, "%p");

    ASSERT_EQ(strcmp(((archi_list_node_named_value_t*)(contexts_data.list.head))->base.name, "1"), 0, int, "%i");
    ASSERT_EQ(((archi_context_t*)((archi_list_node_named_value_t*)contexts_data.list.head)->value.ptr)->handle, &global_context, void*, "%p");
    ASSERT_EQ(strcmp(((archi_list_node_named_value_t*)(contexts_data.list.tail))->base.name, "2"), 0, int, "%i");
    ASSERT_EQ(((archi_context_t*)((archi_list_node_named_value_t*)contexts_data.list.tail)->value.ptr)->handle, &global_context, void*, "%p");

    archi_value_t value = {.num_of = 10};

    step.type = ARCHI_APP_CONFIG_STEP_SET;
    step.as_set = (archi_app_config_step_set_t){
        .key = "1",
        .slot = "plus",
        .value = &value,
    };

    ASSERT_EQ(archi_app_apply_config_step(NULL, &step, &step_data), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context.set_counter, 1, int, "%i");
    ASSERT_EQ(global_context.counter, 10, int, "%i");

    step.as_set.slot = "minus";
    value.num_of = 5;
    ASSERT_EQ(archi_app_apply_config_step(NULL, &step, &step_data), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context.set_counter, 2, int, "%i");
    ASSERT_EQ(global_context.counter, 5, int, "%i");

    step.type = ARCHI_APP_CONFIG_STEP_ASSIGN;
    step.as_assign = (archi_app_config_step_assign_t){
        .destination = {.key = "1", .slot = "minus"},
        .source = {.key = "2", .slot = "value"},
    };

    ASSERT_EQ(archi_app_apply_config_step(NULL, &step, &step_data), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context.set_counter, 3, int, "%i");
    ASSERT_EQ(global_context.get_counter, 1, int, "%i");
    ASSERT_EQ(global_context.counter, 0, int, "%i");

    step.type = ARCHI_APP_CONFIG_STEP_ACT;
    step.as_act = (archi_app_config_step_act_t){
        .key = "1",
        .action = "leet",
    };

    ASSERT_EQ(archi_app_apply_config_step(NULL, &step, &step_data), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context.act_counter, 1, int, "%i");
    ASSERT_EQ(global_context.counter, 1337, int, "%i");

    step.type = ARCHI_APP_CONFIG_STEP_FINAL;
    step.as_final = (archi_app_config_step_final_t){
        .key = "3",
    };

    ASSERT_EQ(archi_app_apply_config_step(NULL, &step, &step_data), 1, archi_status_t, "%i");
    ASSERT_EQ(global_context.final_counter, 0, int, "%i");

    step.as_final.key = "1";
    ASSERT_EQ(archi_app_apply_config_step(NULL, &step, &step_data), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context.final_counter, 1, int, "%i");
    ASSERT_EQ(contexts_data.list.head, contexts_data.list.tail, void*, "%p");

    step.as_final.key = "2";
    ASSERT_EQ(archi_app_apply_config_step(NULL, &step, &step_data), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context.final_counter, 2, int, "%i");
    ASSERT_EQ(contexts_data.list.head, NULL, void*, "%p");
    ASSERT_EQ(contexts_data.list.tail, NULL, void*, "%p");
}

