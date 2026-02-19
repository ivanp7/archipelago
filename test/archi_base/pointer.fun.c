#include "test.h"

#include "archi_base/pointer.fun.h"
#include "archi_base/pointer.def.h"


TEST(ARCHI_POINTER_ATTR_LIMITS)
{
    // Pointer attributes limits
    ASSERT_EQ(ARCHI_POINTER_DATA_SIZE_MAX,      0x00FFFFFFFFFFFFFF, size_t, "%zu");
    ASSERT_EQ(ARCHI_POINTER_DATA_STRIDE_MAX,    0x0080000000000000, size_t, "%zu");
    ASSERT_EQ(ARCHI_POINTER_DATA_ALIGNMENT_MAX, 0x0080000000000000, size_t, "%zu");
    ASSERT_EQ(ARCHI_POINTER_DATA_TAG_MAX,       0x07FFFFFFFFFFFFFF, size_t, "%zu");
    ASSERT_EQ(ARCHI_POINTER_FUNC_TAG_MAX,       0x3FFFFFFFFFFFFFFF, size_t, "%zu");
}

TEST(archi_pointer_attr__pdata)
{
    // Primitive data type attributes
    archi_pointer_attr_t attr;
    archi_error_t error;

    size_t length, stride, alignment;

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(0, 0, 0, &error);
    ASSERT_NE(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(1, 0, 0, &error);
    ASSERT_NE(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(1, 1, 0, &error);
    ASSERT_NE(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(1, 0, 1, &error);
    ASSERT_NE(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(0, 1, 1, &error);
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, 0, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(1, 1, 1, &error);
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, 1, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(ARCHI_POINTER_DATA_SIZE_MAX, 1, 1, &error);
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, ARCHI_POINTER_DATA_SIZE_MAX, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(ARCHI_POINTER_DATA_SIZE_MAX + 1, 1, 1, &error);
    ASSERT_NE(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(0, ARCHI_POINTER_DATA_STRIDE_MAX, 1, &error);
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr & ARCHI_POINTER_TYPE_MASK, 0, archi_pointer_attr_t, "%zX");
    ASSERT_TRUE(archi_pointer_attr_unpk__pdata(attr, &length, &stride, &alignment, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(length, 0, size_t, "%zu");
    ASSERT_EQ(stride, ARCHI_POINTER_DATA_STRIDE_MAX, size_t, "%zu");
    ASSERT_EQ(alignment, 1, size_t, "%zu");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(1, ARCHI_POINTER_DATA_STRIDE_MAX, 1, &error);
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr & ARCHI_POINTER_TYPE_MASK, 0, archi_pointer_attr_t, "%zX");
    ASSERT_TRUE(archi_pointer_attr_unpk__pdata(attr, &length, &stride, &alignment, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(length, 1, size_t, "%zu");
    ASSERT_EQ(stride, ARCHI_POINTER_DATA_STRIDE_MAX, size_t, "%zu");
    ASSERT_EQ(alignment, 1, size_t, "%zu");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(2, ARCHI_POINTER_DATA_STRIDE_MAX, 1, &error);
    ASSERT_NE(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(0, ARCHI_POINTER_DATA_STRIDE_MAX + 1, 1, &error);
    ASSERT_NE(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(0, ARCHI_POINTER_DATA_STRIDE_MAX, ARCHI_POINTER_DATA_ALIGNMENT_MAX, &error);
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr & ARCHI_POINTER_TYPE_MASK, 0, archi_pointer_attr_t, "%zX");
    ASSERT_TRUE(archi_pointer_attr_unpk__pdata(attr, &length, &stride, &alignment, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(length, 0, size_t, "%zu");
    ASSERT_EQ(stride, ARCHI_POINTER_DATA_STRIDE_MAX, size_t, "%zu");
    ASSERT_EQ(alignment, ARCHI_POINTER_DATA_ALIGNMENT_MAX, size_t, "%zu");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(1, ARCHI_POINTER_DATA_STRIDE_MAX, ARCHI_POINTER_DATA_ALIGNMENT_MAX, &error);
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr & ARCHI_POINTER_TYPE_MASK, 0, archi_pointer_attr_t, "%zX");
    ASSERT_TRUE(archi_pointer_attr_unpk__pdata(attr, &length, &stride, &alignment, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(length, 1, size_t, "%zu");
    ASSERT_EQ(stride, ARCHI_POINTER_DATA_STRIDE_MAX, size_t, "%zu");
    ASSERT_EQ(alignment, ARCHI_POINTER_DATA_ALIGNMENT_MAX, size_t, "%zu");

    ARCHI_ERROR_VAR_UNSET(&error);
    attr = archi_pointer_attr__pdata(2, ARCHI_POINTER_DATA_STRIDE_MAX, ARCHI_POINTER_DATA_ALIGNMENT_MAX, &error);
    ASSERT_NE(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");

    for (size_t s = 0; s <= 256; s++)
    {
        for (size_t a = 0; a <= 256; a++)
        {
            ARCHI_ERROR_VAR_UNSET(&error);
            attr = archi_pointer_attr__pdata(16, s, a, &error);

            if ((s == 0) || (a == 0) || ((a & (a - 1)) != 0) || (s % a != 0))
            {
                ASSERT_NE(error.code, 0, archi_error_code_t, "%i");
                ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");
            }
            else
            {
                ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
                ASSERT_EQ(attr & ARCHI_POINTER_TYPE_MASK, 0, archi_pointer_attr_t, "%zX");
                ASSERT_TRUE(archi_pointer_attr_unpk__pdata(attr, &length, &stride, &alignment, &error));
                ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
                ASSERT_EQ(length, 16, size_t, "%zu");
                ASSERT_EQ(stride, s, size_t, "%zu");
                ASSERT_EQ(alignment, a, size_t, "%zu");
            }
        }
    }
}

TEST(archi_pointer_attr__cdata)
{
    // Complex data type attributes
    archi_pointer_attr_t attr;
    archi_error_t error;

    archi_pointer_attr_t tag;

    attr = archi_pointer_attr__cdata(0);
    ASSERT_EQ(attr, 0x3FFFFFFFFFFFFFFF, archi_pointer_attr_t, "%zX");
    ARCHI_ERROR_VAR_UNSET(&error);
    ASSERT_TRUE(archi_pointer_attr_unpk__cdata(attr, &tag, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(tag, 0, archi_pointer_attr_t, "%zX");

    attr = archi_pointer_attr__cdata(1);
    ASSERT_EQ(attr, 0x3FFFFFFFFFFFFFFE, archi_pointer_attr_t, "%zX");
    ARCHI_ERROR_VAR_UNSET(&error);
    ASSERT_TRUE(archi_pointer_attr_unpk__cdata(attr, &tag, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(tag, 1, archi_pointer_attr_t, "%zX");

    attr = archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG_MAX - 1);
    ASSERT_EQ(attr, 0x3800000000000001, archi_pointer_attr_t, "%zX");
    ARCHI_ERROR_VAR_UNSET(&error);
    ASSERT_TRUE(archi_pointer_attr_unpk__cdata(attr, &tag, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(tag, ARCHI_POINTER_DATA_TAG_MAX - 1, archi_pointer_attr_t, "%zX");

    attr = archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG_MAX);
    ASSERT_EQ(attr, 0x3800000000000000, archi_pointer_attr_t, "%zX");
    ARCHI_ERROR_VAR_UNSET(&error);
    ASSERT_TRUE(archi_pointer_attr_unpk__cdata(attr, &tag, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(tag, ARCHI_POINTER_DATA_TAG_MAX, archi_pointer_attr_t, "%zX");

    attr = archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG_MAX + 1);
    ASSERT_EQ(attr, -1, archi_pointer_attr_t, "%zX");

    ASSERT_EQ(archi_pointer_attr__cdata(ARCHI_POINTER_DATA_TAG_MAX) - 1,
            archi_pointer_attr__pdata(1, ARCHI_POINTER_DATA_STRIDE_MAX, 1, NULL),
            archi_pointer_attr_t, "%zX");
}

TEST(archi_pointer_attr__func)
{
    // Function type attributes
    archi_pointer_attr_t attr;
    archi_error_t error;

    archi_pointer_attr_t tag;

    attr = archi_pointer_attr__func(0);
    ASSERT_EQ(attr, 0xC000000000000000, archi_pointer_attr_t, "%zX");
    ARCHI_ERROR_VAR_UNSET(&error);
    ASSERT_TRUE(archi_pointer_attr_unpk__func(attr, &tag, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(tag, 0, archi_pointer_attr_t, "%zX");

    attr = archi_pointer_attr__func(1);
    ASSERT_EQ(attr, 0xC000000000000001, archi_pointer_attr_t, "%zX");
    ARCHI_ERROR_VAR_UNSET(&error);
    ASSERT_TRUE(archi_pointer_attr_unpk__func(attr, &tag, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(tag, 1, archi_pointer_attr_t, "%zX");

    attr = archi_pointer_attr__func(ARCHI_POINTER_FUNC_TAG_MAX - 1);
    ASSERT_EQ(attr, 0xFFFFFFFFFFFFFFFE, archi_pointer_attr_t, "%zX");
    ARCHI_ERROR_VAR_UNSET(&error);
    ASSERT_TRUE(archi_pointer_attr_unpk__func(attr, &tag, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(tag, ARCHI_POINTER_FUNC_TAG_MAX - 1, archi_pointer_attr_t, "%zX");

    attr = archi_pointer_attr__func(ARCHI_POINTER_FUNC_TAG_MAX);
    ASSERT_EQ(attr, 0xFFFFFFFFFFFFFFFF, archi_pointer_attr_t, "%zX");
    ARCHI_ERROR_VAR_UNSET(&error);
    ASSERT_TRUE(archi_pointer_attr_unpk__func(attr, &tag, &error));
    ASSERT_EQ(error.code, 0, archi_error_code_t, "%i");
    ASSERT_EQ(tag, ARCHI_POINTER_FUNC_TAG_MAX, archi_pointer_attr_t, "%zX");

    attr = archi_pointer_attr__func(ARCHI_POINTER_FUNC_TAG_MAX + 1);
    ASSERT_EQ(attr, 0, archi_pointer_attr_t, "%zX");

    ASSERT_EQ(archi_pointer_attr__func(0), ~archi_pointer_attr__cdata(0),
            archi_pointer_attr_t, "%zX");
}

TEST(archi_pointer_attr_compatible)
{
    // Attributes compatibility
    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(256, 16, 8, NULL),
                archi_pointer_attr__pdata(256, 16, 8, NULL)));
    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(256, 16, 8, NULL),
                archi_pointer_attr__pdata(128, 16, 8, NULL)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(128, 16, 8, NULL),
                archi_pointer_attr__pdata(256, 16, 8, NULL)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(256, 16, 4, NULL),
                archi_pointer_attr__pdata(256, 16, 8, NULL)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(256, 16, 8, NULL),
                archi_pointer_attr__pdata(256, 16, 4, NULL)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(256, 8, 8, NULL),
                archi_pointer_attr__pdata(256, 16, 8, NULL)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(256, 16, 8, NULL),
                archi_pointer_attr__pdata(256, 8, 8, NULL)));

    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__cdata(1),
                archi_pointer_attr__cdata(1)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__cdata(1),
                archi_pointer_attr__cdata(2)));
    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__cdata(0),
                archi_pointer_attr__cdata(1)));
    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__cdata(1),
                archi_pointer_attr__cdata(0)));
    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__cdata(0),
                archi_pointer_attr__cdata(0)));

    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__func(1),
                archi_pointer_attr__func(1)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__func(1),
                archi_pointer_attr__func(2)));
    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__func(0),
                archi_pointer_attr__func(1)));
    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__func(1),
                archi_pointer_attr__func(0)));
    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__func(0),
                archi_pointer_attr__func(0)));

    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(256, 16, 8, NULL),
                archi_pointer_attr__cdata(0)));
    ASSERT_TRUE(archi_pointer_attr_compatible(
                archi_pointer_attr__cdata(0),
                archi_pointer_attr__pdata(256, 16, 8, NULL)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(256, 16, 8, NULL),
                archi_pointer_attr__cdata(1)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__cdata(1),
                archi_pointer_attr__pdata(256, 16, 8, NULL)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__pdata(256, 16, 8, NULL),
                archi_pointer_attr__func(0)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__func(0),
                archi_pointer_attr__pdata(256, 16, 8, NULL)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__cdata(0),
                archi_pointer_attr__func(0)));
    ASSERT_FALSE(archi_pointer_attr_compatible(
                archi_pointer_attr__func(0),
                archi_pointer_attr__cdata(0)));
}

