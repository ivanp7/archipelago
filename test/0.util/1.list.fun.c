#include "test.h"

#include "archi/util/list.fun.h"
#include "archi/util/error.def.h"

#include <string.h>

TEST(archi_list_insert_sublist)
{
    archi_list_node_t node[5] = {0};
    archi_list_t list = {0}, sublist = {0};

    ASSERT_EQ(archi_list_insert_sublist(NULL, NULL, NULL, NULL, true),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_list_insert_sublist(&list, NULL, NULL, NULL, true),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_list_insert_sublist(NULL, &sublist, NULL, NULL, true),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_list_insert_sublist(&list, &sublist, NULL, NULL, true),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    sublist.head = &node[0];
    sublist.tail = &node[0];
    ASSERT_EQ(archi_list_insert_sublist(&list, &sublist, NULL, NULL, true),
            0, archi_status_t, "%i");
    ASSERT_EQ(list.head, &node[0], void*, "%p");
    ASSERT_EQ(list.tail, &node[0], void*, "%p");

    sublist.head = &node[1];
    sublist.tail = &node[1];
    ASSERT_EQ(archi_list_insert_sublist(&list, &sublist, NULL, NULL, true),
            0, archi_status_t, "%i");
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[0], void*, "%p");

    sublist.head = &node[2];
    sublist.tail = &node[2];
    ASSERT_EQ(archi_list_insert_sublist(&list, &sublist, NULL, NULL, false),
            0, archi_status_t, "%i");
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.head->next, &node[0], void*, "%p");
    ASSERT_EQ(list.tail->prev, &node[0], void*, "%p");
    ASSERT_EQ(list.tail, &node[2], void*, "%p");

    node[3].next = &node[4];
    node[4].prev = &node[3];
    sublist.head = &node[3];
    sublist.tail = &node[4];

    size_t nth;

    nth = 20;
    ASSERT_EQ(archi_list_insert_sublist(&list, &sublist, archi_list_link_func_select_every_nth, &nth, true),
            1, archi_status_t, "%i");

    nth = 2;
    ASSERT_EQ(archi_list_insert_sublist(&list, &sublist, archi_list_link_func_select_every_nth, &nth, false),
            0, archi_status_t, "%i");

    // 1, 3, 4, 0, 2
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[2], void*, "%p");
    ASSERT_EQ(node[0].prev, &node[4], void*, "%p");
    ASSERT_EQ(node[0].next, &node[2], void*, "%p");
    ASSERT_EQ(node[1].prev, NULL, void*, "%p");
    ASSERT_EQ(node[1].next, &node[3], void*, "%p");
    ASSERT_EQ(node[2].prev, &node[0], void*, "%p");
    ASSERT_EQ(node[2].next, NULL, void*, "%p");
    ASSERT_EQ(node[3].prev, &node[1], void*, "%p");
    ASSERT_EQ(node[3].next, &node[4], void*, "%p");
    ASSERT_EQ(node[4].prev, &node[3], void*, "%p");
    ASSERT_EQ(node[4].next, &node[0], void*, "%p");
}

TEST(archi_list_insert_node)
{
    archi_list_node_t node[5] = {0};
    archi_list_t list = {0};

    ASSERT_EQ(archi_list_insert_node(NULL, NULL, NULL, NULL, true),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_list_insert_node(&list, NULL, NULL, NULL, true),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_list_insert_node(NULL, &node[0], NULL, NULL, true),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    ASSERT_EQ(archi_list_insert_node(&list, &node[0], NULL, NULL, true),
            0, archi_status_t, "%i");
    ASSERT_EQ(list.head, &node[0], void*, "%p");
    ASSERT_EQ(list.tail, &node[0], void*, "%p");

    ASSERT_EQ(archi_list_insert_node(&list, &node[1], NULL, NULL, true),
            0, archi_status_t, "%i");
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[0], void*, "%p");

    ASSERT_EQ(archi_list_insert_node(&list, &node[2], NULL, NULL, false),
            0, archi_status_t, "%i");
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[2], void*, "%p");

    size_t nth;

    nth = 20;
    ASSERT_EQ(archi_list_insert_node(&list, &node[3], archi_list_link_func_select_every_nth, &nth, false),
            1, archi_status_t, "%i");

    nth = 2;
    ASSERT_EQ(archi_list_insert_node(&list, &node[3], archi_list_link_func_select_every_nth, &nth, false),
            0, archi_status_t, "%i");
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[2], void*, "%p");

    nth = 1;
    ASSERT_EQ(archi_list_insert_node(&list, &node[4], archi_list_link_func_select_every_nth, &nth, true),
            0, archi_status_t, "%i");
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[2], void*, "%p");

    // 1, 4, 3, 0, 2
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[2], void*, "%p");
    ASSERT_EQ(node[0].prev, &node[3], void*, "%p");
    ASSERT_EQ(node[0].next, &node[2], void*, "%p");
    ASSERT_EQ(node[1].prev, NULL, void*, "%p");
    ASSERT_EQ(node[1].next, &node[4], void*, "%p");
    ASSERT_EQ(node[2].prev, &node[0], void*, "%p");
    ASSERT_EQ(node[2].next, NULL, void*, "%p");
    ASSERT_EQ(node[3].prev, &node[4], void*, "%p");
    ASSERT_EQ(node[3].next, &node[0], void*, "%p");
    ASSERT_EQ(node[4].prev, &node[1], void*, "%p");
    ASSERT_EQ(node[4].next, &node[3], void*, "%p");
}

TEST(archi_list_cut_sublist)
{
    archi_list_node_t node[5];
    archi_list_t list, sublist;

    node[0].prev = NULL;
    node[0].next = &node[1];
    node[1].prev = &node[0];
    node[1].next = &node[2];
    node[2].prev = &node[1];
    node[2].next = &node[3];
    node[3].prev = &node[2];
    node[3].next = &node[4];
    node[4].prev = &node[3];
    node[4].next = NULL;
    list.head = &node[0];
    list.tail = &node[4];

    sublist.head = sublist.tail = NULL;
    ASSERT_FALSE(archi_list_cut_sublist(&list, &sublist));

    sublist.head = &node[0];
    ASSERT_FALSE(archi_list_cut_sublist(&list, &sublist));

    sublist.head = NULL;
    sublist.tail = &node[4];
    ASSERT_FALSE(archi_list_cut_sublist(&list, &sublist));

    ASSERT_EQ(list.head, &node[0], void*, "%p");
    ASSERT_EQ(list.tail, &node[4], void*, "%p");

    sublist.head = &node[0];
    ASSERT_TRUE(archi_list_cut_sublist(&list, &sublist));
    ASSERT_EQ(list.head, NULL, void*, "%p");
    ASSERT_EQ(list.tail, NULL, void*, "%p");
    ASSERT_EQ(sublist.head, &node[0], void*, "%p");
    ASSERT_EQ(sublist.tail, &node[4], void*, "%p");

    node[0].prev = NULL;
    node[0].next = &node[1];
    node[1].prev = &node[0];
    node[1].next = &node[2];
    node[2].prev = &node[1];
    node[2].next = &node[3];
    node[3].prev = &node[2];
    node[3].next = &node[4];
    node[4].prev = &node[3];
    node[4].next = NULL;
    list.head = &node[0];
    list.tail = &node[4];

    sublist.head = &node[0];
    sublist.tail = &node[1];
    ASSERT_TRUE(archi_list_cut_sublist(&list, &sublist));
    ASSERT_EQ(list.head, &node[2], void*, "%p");
    ASSERT_EQ(list.tail, &node[4], void*, "%p");
    ASSERT_EQ(sublist.head, &node[0], void*, "%p");
    ASSERT_EQ(sublist.tail, &node[1], void*, "%p");
    ASSERT_EQ(node[1].next, NULL, void*, "%p");
    ASSERT_EQ(node[2].prev, NULL, void*, "%p");

    sublist.head = &node[3];
    sublist.tail = &node[4];
    ASSERT_TRUE(archi_list_cut_sublist(&list, &sublist));
    ASSERT_EQ(list.head, &node[2], void*, "%p");
    ASSERT_EQ(list.tail, &node[2], void*, "%p");
    ASSERT_EQ(sublist.head, &node[3], void*, "%p");
    ASSERT_EQ(sublist.tail, &node[4], void*, "%p");
    ASSERT_EQ(node[2].next, NULL, void*, "%p");
    ASSERT_EQ(node[3].prev, NULL, void*, "%p");

    sublist.head = &node[2];
    sublist.tail = &node[2];
    ASSERT_TRUE(archi_list_cut_sublist(&list, &sublist));
    ASSERT_EQ(list.head, NULL, void*, "%p");
    ASSERT_EQ(list.tail, NULL, void*, "%p");
    ASSERT_EQ(sublist.head, &node[2], void*, "%p");
    ASSERT_EQ(sublist.tail, &node[2], void*, "%p");
    ASSERT_EQ(node[2].next, NULL, void*, "%p");
    ASSERT_EQ(node[2].prev, NULL, void*, "%p");
}

TEST(archi_list_cut_node)
{
    archi_list_node_t node[5];
    archi_list_t list;

    node[0].prev = NULL;
    node[0].next = &node[1];
    node[1].prev = &node[0];
    node[1].next = &node[2];
    node[2].prev = &node[1];
    node[2].next = &node[3];
    node[3].prev = &node[2];
    node[3].next = &node[4];
    node[4].prev = &node[3];
    node[4].next = NULL;
    list.head = &node[0];
    list.tail = &node[4];

    ASSERT_FALSE(archi_list_cut_node(&list, NULL));

    ASSERT_TRUE(archi_list_cut_node(NULL, &node[3]));
    ASSERT_EQ(node[3].next, NULL, void*, "%p");
    ASSERT_EQ(node[3].prev, NULL, void*, "%p");

    ASSERT_TRUE(archi_list_cut_node(&list, &node[0]));
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[4], void*, "%p");
    ASSERT_EQ(node[0].next, NULL, void*, "%p");
    ASSERT_EQ(node[1].prev, NULL, void*, "%p");

    ASSERT_TRUE(archi_list_cut_node(&list, &node[4]));
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[2], void*, "%p");
    ASSERT_EQ(node[3].next, NULL, void*, "%p");

    ASSERT_TRUE(archi_list_cut_node(&list, &node[2]));
    ASSERT_EQ(list.head, &node[1], void*, "%p");
    ASSERT_EQ(list.tail, &node[1], void*, "%p");
    ASSERT_EQ(node[2].prev, NULL, void*, "%p");
    ASSERT_EQ(node[1].next, node[3].prev, void*, "%p");

    ASSERT_TRUE(archi_list_cut_node(&list, &node[1]));
    ASSERT_EQ(list.head, NULL, void*, "%p");
    ASSERT_EQ(list.tail, NULL, void*, "%p");
    ASSERT_EQ(node[1].next, NULL, void*, "%p");
    ASSERT_EQ(node[1].prev, NULL, void*, "%p");
}

TEST(archi_list_remove_nodes)
{
    archi_list_node_t node[5];
    archi_list_t list = {0};

    node[0].prev = NULL;
    node[0].next = &node[1];
    node[1].prev = &node[0];
    node[1].next = &node[2];
    node[2].prev = &node[1];
    node[2].next = &node[3];
    node[3].prev = &node[2];
    node[3].next = &node[4];
    node[4].prev = &node[3];
    node[4].next = NULL;

    size_t num_removed = 0;

    size_t nth = 2;
    ASSERT_EQ(archi_list_remove_nodes(NULL, archi_list_node_func_select_every_nth, &nth,
                NULL, NULL, true, 0, &num_removed),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(num_removed, 0, unsigned, "%u");

    list.head = NULL;
    list.tail = &node[4];
    ASSERT_EQ(archi_list_remove_nodes(&list, archi_list_node_func_select_every_nth, &nth,
                NULL, NULL, true, 0, &num_removed),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(num_removed, 0, unsigned, "%u");

    list.head = &node[0];
    list.tail = NULL;
    ASSERT_EQ(archi_list_remove_nodes(&list, archi_list_node_func_select_every_nth, &nth,
                NULL, NULL, false, 0, &num_removed),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(num_removed, 0, unsigned, "%u");

    list.head = &node[0];
    list.tail = &node[4];

    nth = 4;
    ASSERT_EQ(archi_list_remove_nodes(&list, archi_list_node_func_select_every_nth, &nth,
                NULL, NULL, true, 0, &num_removed),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_removed, 1, unsigned, "%u");
    ASSERT_EQ(list.tail, &node[3], void*, "%p");
    ASSERT_EQ(node[3].next, NULL, void*, "%p");
    ASSERT_EQ(node[4].prev, NULL, void*, "%p");

    nth = 1;
    ASSERT_EQ(archi_list_remove_nodes(&list, archi_list_node_func_select_every_nth, &nth,
                NULL, NULL, true, 0, &num_removed),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_removed, 2, unsigned, "%u");
    ASSERT_EQ(list.tail, &node[2], void*, "%p");
    ASSERT_EQ(node[0].next, &node[2], void*, "%p");
    ASSERT_EQ(node[2].prev, &node[0], void*, "%p");

    nth = 0;
    ASSERT_EQ(archi_list_remove_nodes(&list, archi_list_node_func_select_every_nth, &nth,
                NULL, NULL, false, 0, &num_removed),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_removed, 2, unsigned, "%u");
    ASSERT_EQ(list.head, NULL, void*, "%p");
    ASSERT_EQ(list.tail, NULL, void*, "%p");

    node[0].prev = NULL;
    node[0].next = &node[1];
    node[1].prev = &node[0];
    node[1].next = &node[2];
    node[2].prev = &node[1];
    node[2].next = &node[3];
    node[3].prev = &node[2];
    node[3].next = &node[4];
    node[4].prev = &node[3];
    node[4].next = NULL;
    list.head = &node[0];
    list.tail = &node[4];

    nth = 0;
    ASSERT_EQ(archi_list_remove_nodes(&list, archi_list_node_func_select_every_nth, &nth,
                NULL, NULL, false, 3, &num_removed),
            1, archi_status_t, "%i");
    ASSERT_EQ(num_removed, 3, unsigned, "%u");
    ASSERT_EQ(list.tail, &node[1], void*, "%p");
    ASSERT_EQ(node[1].next, NULL, void*, "%p");
    ASSERT_EQ(node[2].prev, NULL, void*, "%p");
}

static
ARCHI_LIST_ACT_FUNC(flip_bit)
{
    (void) node;
    size_t *mask = data;
    *mask |= (size_t)1 << position;
    return 0;
}

TEST(archi_list_traverse)
{
    archi_list_node_t node[5];
    archi_list_t list = {.head = &node[0], .tail = &node[4]};

    node[0].prev = NULL;
    node[0].next = &node[1];
    node[1].prev = &node[0];
    node[1].next = &node[2];
    node[2].prev = &node[1];
    node[2].next = &node[3];
    node[3].prev = &node[2];
    node[3].next = &node[4];
    node[4].prev = &node[3];
    node[4].next = NULL;

    size_t num_counted = 0;
    size_t mask = 0;

    size_t nth = 0;
    ASSERT_EQ(archi_list_traverse(NULL, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, true, 0, &num_counted),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 0, unsigned, "%u");

    list.head = NULL;
    list.tail = &node[4];
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, true, 0, &num_counted),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 0, unsigned, "%u");

    list.head = &node[0];
    list.tail = NULL;
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, false, 0, &num_counted),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 0, unsigned, "%u");

    list.head = &node[0];
    list.tail = &node[4];

    nth = 0;
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, true, 0, &num_counted),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 5, unsigned, "%u");
    ASSERT_EQ(mask, 0x1F, unsigned, "%u");

    nth = 0;
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, false, 0, &num_counted),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 5, unsigned, "%u");
    ASSERT_EQ(mask, 0x1F, unsigned, "%u");

    mask = 0;
    nth = 1;
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, true, 0, &num_counted),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 2, unsigned, "%u");
    ASSERT_EQ(mask, 0x0A, unsigned, "%u");

    mask = 0;
    nth = 1;
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, false, 0, &num_counted),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 2, unsigned, "%u");
    ASSERT_EQ(mask, 0x0A, unsigned, "%u");

    mask = 0;
    nth = 1;
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, true, 1, &num_counted),
            1, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 1, unsigned, "%u");
    ASSERT_EQ(mask, 0x02, unsigned, "%u");

    mask = 0;
    nth = 2;
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, true, 0, &num_counted),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 1, unsigned, "%u");
    ASSERT_EQ(mask, 0x04, unsigned, "%u");

    mask = 0;
    nth = 4;
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, true, 0, &num_counted),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 1, unsigned, "%u");
    ASSERT_EQ(mask, 0x10, unsigned, "%u");

    mask = 0;
    nth = 5;
    ASSERT_EQ(archi_list_traverse(&list, archi_list_node_func_select_every_nth, &nth,
                flip_bit, &mask, false, 0, &num_counted),
            0, archi_status_t, "%i");
    ASSERT_EQ(num_counted, 0, unsigned, "%u");
    ASSERT_EQ(mask, 0x00, unsigned, "%u");
}

TEST(archi_list_link_func_select_every_nth)
{
    size_t nth;

    nth = 0;
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 0, false, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 0, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 1, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 2, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 3, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 4, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 5, false, &nth),
            0, archi_status_t, "%i");

    nth = 1;
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 0, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 1, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 2, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 3, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 4, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 5, false, &nth),
            0, archi_status_t, "%i");

    nth = 2;
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 0, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 1, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 2, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 3, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 4, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 5, false, &nth),
            0, archi_status_t, "%i");

    nth = 5;
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 0, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 1, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 2, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 3, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 4, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 5, false, &nth),
            0, archi_status_t, "%i");

    nth = 6;
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 0, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 1, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 2, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 3, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 4, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_link_func_select_every_nth(NULL, NULL, 5, false, &nth),
            1, archi_status_t, "%i");
}

TEST(archi_list_node_func_select_every_nth)
{
    size_t nth;

    nth = 0;
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 0, false, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");

    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 0, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 1, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 2, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 3, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 4, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 5, false, &nth),
            0, archi_status_t, "%i");

    nth = 1;
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 0, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 1, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 2, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 3, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 4, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 5, false, &nth),
            0, archi_status_t, "%i");

    nth = 2;
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 0, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 1, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 2, false, &nth),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 3, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 4, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 5, false, &nth),
            0, archi_status_t, "%i");

    nth = 5;
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 0, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 1, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 2, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 3, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 4, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 5, false, &nth),
            0, archi_status_t, "%i");

    nth = 6;
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 0, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 1, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 2, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 3, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 4, false, &nth),
            1, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_every_nth(NULL, 5, false, &nth),
            1, archi_status_t, "%i");
}

TEST(archi_list_node_func_select_by_name)
{
    archi_list_node_named_t node = {0};
    archi_list_node_t *node_ptr = (archi_list_node_t*)&node;

    ASSERT_EQ(archi_list_node_func_select_by_name(node_ptr, 0, false, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    node.name = "abc";
    ASSERT_EQ(archi_list_node_func_select_by_name(node_ptr, 0, false, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_by_name(node_ptr, 0, false, "abc"),
            0, archi_status_t, "%i");
    ASSERT_EQ(archi_list_node_func_select_by_name(node_ptr, 0, false, "def"),
            1, archi_status_t, "%i");
}

TEST(archi_list_act_func_extract_node)
{
    archi_list_node_t node, *node_ptr;

    ASSERT_EQ(archi_list_act_func_extract_node(&node, 0, NULL),
            ARCHI_ERROR_MISUSE, archi_status_t, "%i");
    ASSERT_EQ(archi_list_act_func_extract_node(&node, 0, &node_ptr),
            0, archi_status_t, "%i");
    ASSERT_EQ(node_ptr, &node, void*, "%p");
}

static
ARCHI_CONTAINER_ELEMENT_FUNC(flip_bit2)
{
    (void) element;

    size_t *mask = data;
    *mask |= (size_t)1 << (((const char*)key)[0] - '0');
    return 0;
}

TEST(archi_list_container_interface)
{
    archi_list_container_data_t container_data = {0};
    void *element;

    // insert
    ASSERT_EQ(archi_list_container_insert(&container_data, "1", "abc"),
            0, archi_status_t, "%i");
    ASSERT_EQ(container_data.list.head, container_data.list.tail, void*, "%p");
    ASSERT_EQ(strcmp(((archi_list_node_named_t*)container_data.list.head)->name, "1"), 0, int, "%i");
    ASSERT_EQ(strcmp(((archi_list_node_named_value_t*)container_data.list.head)->value.ptr, "abc"), 0, int, "%i");

    ASSERT_EQ(container_data.list.head->prev, NULL, void*, "%p");
    ASSERT_EQ(container_data.list.head->next, NULL, void*, "%p");
    ASSERT_EQ(container_data.list.tail->prev, NULL, void*, "%p");
    ASSERT_EQ(container_data.list.tail->next, NULL, void*, "%p");

    ASSERT_EQ(archi_list_container_insert(&container_data, "2", "def"),
            0, archi_status_t, "%i");
    ASSERT_NE(container_data.list.head, container_data.list.tail, void*, "%p");
    ASSERT_EQ(strcmp(((archi_list_node_named_t*)container_data.list.tail)->name, "2"), 0, int, "%i");
    ASSERT_EQ(strcmp(((archi_list_node_named_value_t*)container_data.list.tail)->value.ptr, "def"), 0, int, "%i");

    ASSERT_EQ(container_data.list.head->next, container_data.list.tail, void*, "%p");
    ASSERT_EQ(container_data.list.tail->prev, container_data.list.head, void*, "%p");
    ASSERT_EQ(container_data.list.head->prev, NULL, void*, "%p");
    ASSERT_EQ(container_data.list.tail->next, NULL, void*, "%p");

    container_data.insert_to_head = true;

    ASSERT_EQ(archi_list_container_insert(&container_data, "3", "ghi"),
            0, archi_status_t, "%i");
    ASSERT_EQ(strcmp(((archi_list_node_named_t*)container_data.list.head)->name, "3"), 0, int, "%i");
    ASSERT_EQ(strcmp(((archi_list_node_named_value_t*)container_data.list.head)->value.ptr, "ghi"), 0, int, "%i");

    ASSERT_EQ(container_data.list.head->next->next, container_data.list.tail, void*, "%p");
    ASSERT_EQ(container_data.list.tail->prev->prev, container_data.list.head, void*, "%p");

    // extract
    ASSERT_EQ(archi_list_container_extract(&container_data, "4", &element),
            1, archi_status_t, "%i");

    ASSERT_EQ(archi_list_container_extract(&container_data, "3", &element),
            0, archi_status_t, "%i");
    ASSERT_EQ(strcmp(element, "ghi"), 0, int, "%i");

    ASSERT_EQ(archi_list_container_extract(&container_data, "2", &element),
            0, archi_status_t, "%i");
    ASSERT_EQ(strcmp(element, "def"), 0, int, "%i");

    container_data.traverse_from_head = true;

    ASSERT_EQ(archi_list_container_extract(&container_data, "1", &element),
            0, archi_status_t, "%i");
    ASSERT_EQ(strcmp(element, "abc"), 0, int, "%i");

    container_data.traverse_from_head = false;

    // traverse
    size_t mask = 0;
    ASSERT_EQ(archi_list_container_traverse(&container_data, flip_bit2, &mask),
            0, archi_status_t, "%i");
    ASSERT_EQ(mask, 0xE, unsigned, "%u");

    container_data.traverse_from_head = true;

    mask = 0;
    ASSERT_EQ(archi_list_container_traverse(&container_data, flip_bit2, &mask),
            0, archi_status_t, "%i");
    ASSERT_EQ(mask, 0xE, unsigned, "%u");

    container_data.traverse_from_head = false;

    // remove
    ASSERT_EQ(archi_list_container_remove(&container_data, "4", &element),
            1, archi_status_t, "%i");

    ASSERT_EQ(archi_list_container_remove(&container_data, "3", &element),
            0, archi_status_t, "%i");
    ASSERT_NE(strcmp(((archi_list_node_named_t*)container_data.list.head)->name, "3"), 0, int, "%i");
    ASSERT_EQ(strcmp(element, "ghi"), 0, int, "%i");

    container_data.traverse_from_head = true;

    ASSERT_EQ(archi_list_container_remove(&container_data, "1", &element),
            0, archi_status_t, "%i");
    ASSERT_EQ(container_data.list.head, container_data.list.tail, void*, "%p");
    ASSERT_NE(strcmp(((archi_list_node_named_t*)container_data.list.head)->name, "1"), 0, int, "%i");
    ASSERT_EQ(strcmp(element, "abc"), 0, int, "%i");

    ASSERT_EQ(archi_list_container_remove(&container_data, "2", &element),
            0, archi_status_t, "%i");
    ASSERT_EQ(container_data.list.head, container_data.list.tail, void*, "%p");
    ASSERT_EQ(container_data.list.head, NULL, void*, "%p");
    ASSERT_EQ(strcmp(element, "def"), 0, int, "%i");
}

