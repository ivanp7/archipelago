#include "test.h"

#include "archi/app/config.fun.h"
#include "archi/app/config.typ.h"
#include "archi/app/context.typ.h"
#include "archi/app/instance.typ.h"
#include "archi/util/container.fun.h"
#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <string.h>

#define INTERFACE "iface"

static
ARCHI_CONTEXT_INIT_FUNC(init_func);
static
ARCHI_CONTEXT_FINAL_FUNC(final_func);
static
ARCHI_CONTEXT_SET_FUNC(set_func);
static
ARCHI_CONTEXT_GET_FUNC(get_func);
static
ARCHI_CONTEXT_ACT_FUNC(act_func);

static
archi_context_interface_t global_context_interface = {
    .init_fn = init_func,
    .final_fn = final_func,
    .set_fn = set_func,
    .get_fn = get_func,
    .act_fn = act_func,
};

struct context {
    int counter;

    int init_counter;
    int final_counter;
    int set_counter;
    int get_counter;
    int act_counter;
};

static
struct context global_context_data;

static
ARCHI_CONTEXT_INIT_FUNC(init_func)
{
    (void) config;
    *context = &global_context_data;
    global_context_data.init_counter++;
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

TEST(archi_app_add_context)
{
    global_context_data = (struct context){0};

    archi_list_container_data_t contexts_data = {.insert_to_head = false, .traverse_from_head = false};

    archi_application_t app = {
        .contexts = {.data = &contexts_data, .interface = &archi_list_container_interface},
    };

    ASSERT_EQ(archi_app_add_context(&app, "1", &global_context_interface, NULL), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.init_counter, 1, int, "%i");

    ASSERT_EQ(archi_app_add_context(&app, "2", &global_context_interface, NULL), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.init_counter, 2, int, "%i");

    ASSERT_NE(contexts_data.list.head, NULL, void*, "%p");
    ASSERT_NE(contexts_data.list.tail, NULL, void*, "%p");
    ASSERT_EQ(contexts_data.list.head->next, contexts_data.list.tail, void*, "%p");
    ASSERT_EQ(contexts_data.list.tail->prev, contexts_data.list.head, void*, "%p");

    ASSERT_EQ(strcmp(((archi_list_node_named_value_t*)(contexts_data.list.head))->base.name, "1"), 0, int, "%i");
    ASSERT_EQ(((archi_context_t*)((archi_list_node_named_value_t*)contexts_data.list.head)->value.ptr)->handle, &global_context_data, void*, "%p");
    ASSERT_EQ(strcmp(((archi_list_node_named_value_t*)(contexts_data.list.tail))->base.name, "2"), 0, int, "%i");
    ASSERT_EQ(((archi_context_t*)((archi_list_node_named_value_t*)contexts_data.list.tail)->value.ptr)->handle, &global_context_data, void*, "%p");

    ASSERT_EQ(archi_app_remove_context(&app, "1"), 0, archi_status_t, "%i");
    ASSERT_EQ(contexts_data.list.head, contexts_data.list.tail, void*, "%p");
    ASSERT_EQ(contexts_data.list.head->next, NULL, void*, "%p");
    ASSERT_EQ(contexts_data.list.tail->prev, NULL, void*, "%p");

    ASSERT_EQ(archi_app_remove_context(&app, "2"), 0, archi_status_t, "%i");
    ASSERT_EQ(contexts_data.list.head, NULL, void*, "%p");
    ASSERT_EQ(contexts_data.list.tail, NULL, void*, "%p");

    ASSERT_EQ(archi_app_remove_context(&app, "1"), 1, archi_status_t, "%i");
}

TEST(archi_app_remove_context)
{
    NOTHING; // see TEST(archi_app_add_context)
}

TEST(archi_app_do_config_step)
{
    global_context_data = (struct context){0};

    archi_list_node_named_value_t context_interface_node = {
        .base.name = INTERFACE,
        .value = {.ptr = &global_context_interface, .num_of = 1, .type = ARCHI_VALUE_DATA},
    };

    archi_list_container_data_t context_interfaces_data = {
        .list = {(archi_list_node_t*)&context_interface_node, (archi_list_node_t*)&context_interface_node},
        .traverse_from_head = true,
    };
    archi_list_container_data_t contexts_data = {.insert_to_head = false, .traverse_from_head = false};

    archi_application_t app = {
        .interfaces = {.data = &context_interfaces_data, .interface = &archi_list_container_interface},
        .contexts = {.data = &contexts_data, .interface = &archi_list_container_interface},
    };

    archi_app_config_step_t step;

    step.type = ARCHI_APP_CONFIG_STEP_INIT;
    step.key = "1";
    step.as_init = (archi_app_config_step_init_t){
        .interface_key = INTERFACE,
    };

    ASSERT_EQ(archi_app_do_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.init_counter, 1, int, "%i");

    step.key = "2";
    ASSERT_EQ(archi_app_do_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.init_counter, 2, int, "%i");

    ASSERT_NE(contexts_data.list.head, NULL, void*, "%p");
    ASSERT_NE(contexts_data.list.tail, NULL, void*, "%p");
    ASSERT_EQ(contexts_data.list.head->next, contexts_data.list.tail, void*, "%p");
    ASSERT_EQ(contexts_data.list.tail->prev, contexts_data.list.head, void*, "%p");

    ASSERT_EQ(strcmp(((archi_list_node_named_value_t*)(contexts_data.list.head))->base.name, "1"), 0, int, "%i");
    ASSERT_EQ(((archi_context_t*)((archi_list_node_named_value_t*)contexts_data.list.head)->value.ptr)->handle, &global_context_data, void*, "%p");
    ASSERT_EQ(strcmp(((archi_list_node_named_value_t*)(contexts_data.list.tail))->base.name, "2"), 0, int, "%i");
    ASSERT_EQ(((archi_context_t*)((archi_list_node_named_value_t*)contexts_data.list.tail)->value.ptr)->handle, &global_context_data, void*, "%p");

    archi_value_t value = {.num_of = 10};

    step.type = ARCHI_APP_CONFIG_STEP_SET;
    step.key = "1";
    step.as_set = (archi_app_config_step_set_t){
        .slot = "plus",
        .value = &value,
    };

    ASSERT_EQ(archi_app_do_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.set_counter, 1, int, "%i");
    ASSERT_EQ(global_context_data.counter, 10, int, "%i");

    step.as_set.slot = "minus";
    value.num_of = 5;
    ASSERT_EQ(archi_app_do_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.set_counter, 2, int, "%i");
    ASSERT_EQ(global_context_data.counter, 5, int, "%i");

    step.type = ARCHI_APP_CONFIG_STEP_ASSIGN;
    step.key = "1";
    step.as_assign = (archi_app_config_step_assign_t){
        .slot = "minus",
        .source_key = "2",
        .source_slot = "value",
    };

    ASSERT_EQ(archi_app_do_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.set_counter, 3, int, "%i");
    ASSERT_EQ(global_context_data.get_counter, 1, int, "%i");
    ASSERT_EQ(global_context_data.counter, 0, int, "%i");

    step.type = ARCHI_APP_CONFIG_STEP_ACT;
    step.key = "1";
    step.as_act = (archi_app_config_step_act_t){
        .action = "leet",
    };

    ASSERT_EQ(archi_app_do_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.act_counter, 1, int, "%i");
    ASSERT_EQ(global_context_data.counter, 1337, int, "%i");

    step.type = ARCHI_APP_CONFIG_STEP_FINAL;
    step.key = "3";

    ASSERT_EQ(archi_app_do_config_step(&app, step), 1, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.final_counter, 0, int, "%i");

    step.key = "1";
    ASSERT_EQ(archi_app_do_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.final_counter, 1, int, "%i");
    ASSERT_EQ(contexts_data.list.head, contexts_data.list.tail, void*, "%p");

    step.key = "2";
    ASSERT_EQ(archi_app_do_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.final_counter, 2, int, "%i");
    ASSERT_EQ(contexts_data.list.head, NULL, void*, "%p");
    ASSERT_EQ(contexts_data.list.tail, NULL, void*, "%p");
}

TEST(archi_app_undo_config_step)
{
    global_context_data = (struct context){0};

    archi_list_container_data_t contexts_data = {
        .insert_to_head = false, .traverse_from_head = false};

    archi_application_t app = {
        .contexts = {.data = &contexts_data, .interface = &archi_list_container_interface},
    };

    ASSERT_EQ(archi_app_add_context(&app, "1", &global_context_interface, NULL), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.init_counter, 1, int, "%i");

    archi_app_config_step_t step;

    step.type = ARCHI_APP_CONFIG_STEP_FINAL;

    step.key = "1";
    ASSERT_EQ(archi_app_undo_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.init_counter, 1, int, "%i");
    ASSERT_EQ(global_context_data.final_counter, 0, int, "%i");
    ASSERT_EQ(contexts_data.list.head, contexts_data.list.tail, void*, "%p");

    step.type = ARCHI_APP_CONFIG_STEP_INIT;

    step.key = "2";
    ASSERT_EQ(archi_app_undo_config_step(&app, step), 1, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.init_counter, 1, int, "%i");
    ASSERT_EQ(global_context_data.final_counter, 0, int, "%i");
    ASSERT_EQ(contexts_data.list.head, contexts_data.list.tail, void*, "%p");

    step.key = "1";
    ASSERT_EQ(archi_app_undo_config_step(&app, step), 0, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.init_counter, 1, int, "%i");
    ASSERT_EQ(global_context_data.final_counter, 1, int, "%i");
    ASSERT_EQ(contexts_data.list.head, NULL, void*, "%p");
    ASSERT_EQ(contexts_data.list.tail, NULL, void*, "%p");

    step.key = "2";
    ASSERT_EQ(archi_app_undo_config_step(&app, step), 1, archi_status_t, "%i");
    ASSERT_EQ(global_context_data.init_counter, 1, int, "%i");
    ASSERT_EQ(global_context_data.final_counter, 1, int, "%i");
    ASSERT_EQ(contexts_data.list.head, NULL, void*, "%p");
    ASSERT_EQ(contexts_data.list.tail, NULL, void*, "%p");
}

