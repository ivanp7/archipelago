#include "test.h"

#include "archi/util/container.fun.h"
#include "archi/util/error.def.h"

#include <string.h>

static
ARCHI_CONTAINER_INSERT_FUNC(insert_func)
{
    if ((key == NULL) || (element == NULL))
        return 1;

    int *counter = container;

    if ((strcmp(key, "correct_key") == 0) && (strcmp(element, "right_element") == 0))
        (*counter)++;

    return 0;
}

TEST(archi_container_insert)
{
    int counter = 0;

    archi_container_interface_t interface = {.insert_fn = insert_func};
    archi_container_t container = {.data = &counter, .interface = &interface};

    ASSERT_EQ(archi_container_insert((archi_container_t){.data = &counter}, "correct_key", "right_element"),
            ARCHI_ERROR_MISUSE, archi_status_t, "%u");
    ASSERT_EQ(archi_container_insert((archi_container_t){.interface = &interface}, "correct_key", "right_element"),
            ARCHI_ERROR_MISUSE, archi_status_t, "%u");
    {
        archi_container_interface_t interface = {0};
        ASSERT_EQ(archi_container_insert((archi_container_t){.data = &counter, .interface = &interface}, "correct_key", "right_element"),
                ARCHI_ERROR_FUNCTION, archi_status_t, "%u");
    }

    ASSERT_EQ(archi_container_insert(container, NULL, "right_element"),
            1, archi_status_t, "%u");
    ASSERT_EQ(archi_container_insert(container, "correct_key", NULL),
            1, archi_status_t, "%u");

    ASSERT_EQ(counter, 0, int, "%i");
    ASSERT_EQ(archi_container_insert(container, "correct_key", "right_element"),
            0, archi_status_t, "%u");
    ASSERT_EQ(counter, 1, int, "%i");
}

static
ARCHI_CONTAINER_REMOVE_FUNC(remove_func)
{
    if ((key == NULL) || (element == NULL))
        return 1;

    int *counter = container;

    if (strcmp(key, "correct_key") == 0)
    {
        (*counter)++;
        *element = "right_element";
    }

    return 0;
}

TEST(archi_container_remove)
{
    int counter = 0;

    archi_container_interface_t interface = {.remove_fn = remove_func};
    archi_container_t container = {.data = &counter, .interface = &interface};

    const char *element = NULL;

    ASSERT_EQ(archi_container_remove((archi_container_t){.data = &counter}, "correct_key", (void**)&element),
            ARCHI_ERROR_MISUSE, archi_status_t, "%u");
    ASSERT_EQ(element, NULL, const void*, "%p");
    ASSERT_EQ(archi_container_remove((archi_container_t){.interface = &interface}, "correct_key", (void**)&element),
            ARCHI_ERROR_MISUSE, archi_status_t, "%u");
    ASSERT_EQ(element, NULL, const void*, "%p");
    {
        archi_container_interface_t interface = {0};
        ASSERT_EQ(archi_container_remove((archi_container_t){.data = &counter, .interface = &interface}, "correct_key", (void**)&element),
                ARCHI_ERROR_FUNCTION, archi_status_t, "%u");
        ASSERT_EQ(element, NULL, const void*, "%p");
    }

    ASSERT_EQ(archi_container_remove(container, NULL, (void**)&element),
            1, archi_status_t, "%u");
    ASSERT_EQ(element, NULL, const void*, "%p");

    ASSERT_EQ(counter, 0, int, "%i");
    ASSERT_EQ(archi_container_remove(container, "correct_key", (void**)&element),
            0, archi_status_t, "%u");
    ASSERT_EQ(counter, 1, int, "%i");
    ASSERT_NE(element, NULL, const void*, "%p");
    if (element != NULL)
        ASSERT_EQ(strcmp(element, "right_element"), 0, int, "%i");
}

static
ARCHI_CONTAINER_EXTRACT_FUNC(extract_func)
{
    if ((key == NULL) || (element == NULL))
        return 1;

    int *counter = container;

    if (strcmp(key, "correct_key") == 0)
    {
        (*counter)++;
        *element = "right_element";
    }

    return 0;
}

TEST(archi_container_extract)
{
    int counter = 0;

    archi_container_interface_t interface = {.extract_fn = extract_func};
    archi_container_t container = {.data = &counter, .interface = &interface};

    const char *element = NULL;

    ASSERT_EQ(archi_container_extract((archi_container_t){.data = &counter}, "correct_key", (void**)&element),
            ARCHI_ERROR_MISUSE, archi_status_t, "%u");
    ASSERT_EQ(element, NULL, const void*, "%p");
    ASSERT_EQ(archi_container_extract((archi_container_t){.interface = &interface}, "correct_key", (void**)&element),
            ARCHI_ERROR_MISUSE, archi_status_t, "%u");
    ASSERT_EQ(element, NULL, const void*, "%p");
    {
        archi_container_interface_t interface = {0};
        ASSERT_EQ(archi_container_extract((archi_container_t){.data = &counter, .interface = &interface}, "correct_key", (void**)&element),
                ARCHI_ERROR_FUNCTION, archi_status_t, "%u");
        ASSERT_EQ(element, NULL, const void*, "%p");
    }

    ASSERT_EQ(archi_container_extract(container, NULL, (void**)&element),
            1, archi_status_t, "%u");
    ASSERT_EQ(element, NULL, const void*, "%p");

    ASSERT_EQ(counter, 0, int, "%i");
    ASSERT_EQ(archi_container_extract(container, "correct_key", (void**)&element),
            0, archi_status_t, "%u");
    ASSERT_EQ(counter, 1, int, "%i");
    ASSERT_NE(element, NULL, const void*, "%p");
    if (element != NULL)
        ASSERT_EQ(strcmp(element, "right_element"), 0, int, "%i");
}

static
ARCHI_CONTAINER_ELEMENT_FUNC(element_func)
{
    if ((key == NULL) || (element == NULL) || (data == NULL))
        return 1;

    int *counter = data;

    if ((strcmp(key, "correct_key") == 0) && (strcmp(element, "right_element") == 0))
        (*counter)++;

    return 0;
}

static
ARCHI_CONTAINER_TRAVERSE_FUNC(traverse_func)
{
    (void) container;

    func("correct_key", "right_element", func_data);
    return func("correct_key", NULL, func_data);
}

TEST(archi_container_traverse)
{
    int counter = 0;

    archi_container_interface_t interface = {.traverse_fn = traverse_func};
    archi_container_t container = {.data = &counter, .interface = &interface};

    ASSERT_EQ(archi_container_traverse((archi_container_t){.data = &counter}, element_func, &counter),
            ARCHI_ERROR_MISUSE, archi_status_t, "%u");
    ASSERT_EQ(archi_container_traverse((archi_container_t){.interface = &interface}, element_func, &counter),
            ARCHI_ERROR_MISUSE, archi_status_t, "%u");
    {
        archi_container_interface_t interface = {0};
        ASSERT_EQ(archi_container_traverse((archi_container_t){.data = &counter, .interface = &interface}, element_func, &counter),
                ARCHI_ERROR_FUNCTION, archi_status_t, "%u");
    }
    ASSERT_EQ(archi_container_traverse(container, NULL, &counter),
            ARCHI_ERROR_MISUSE, archi_status_t, "%u");

    ASSERT_EQ(counter, 0, int, "%i");
    ASSERT_EQ(archi_container_traverse(container, element_func, &counter),
            1, archi_status_t, "%u");
    ASSERT_EQ(counter, 1, int, "%i");
}

