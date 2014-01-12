#include <zcut.h>
#include <stdlib.h>

/**
 * test_expect_suite
 */
TEST_CASE(test_expect_passed)
{
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);

    EXPECT_EQ(0, 0);
    EXPECT_NE(0, 1);
    EXPECT_LT(0, 1);
    EXPECT_LE(1, 1);
    EXPECT_GT(1, 0);
    EXPECT_GE(1, 1);

    EXPECT_STR_EQ("a", "a");
    EXPECT_STR_NE("a", "b");
    EXPECT_STR_LT("a", "b");
    EXPECT_STR_LE("b", "b");
    EXPECT_STR_GT("b", "a");
    EXPECT_STR_GE("b", "b");

    EXPECT_STR_IC_EQ("A", "a");
    EXPECT_STR_IC_NE("A", "b");
    EXPECT_STR_IC_LT("A", "b");
    EXPECT_STR_IC_LE("B", "b");
    EXPECT_STR_IC_GT("B", "a");
    EXPECT_STR_IC_GE("B", "b");
}

TEST_CASE(test_expect_failed)
{
    EXPECT_TRUE(false, "error bool");
    EXPECT_FALSE(true, "error bool");

    EXPECT_EQ(0, 1, "error int:%d, %d", 0, 1);
    EXPECT_NE(1, 1, "error int:%s, %s, %d", __FILE__, __func__, __LINE__);
    EXPECT_LT(1, 1);
    EXPECT_LE(1, 0);
    EXPECT_GT(1, 1);
    EXPECT_GE(0, 1);

    EXPECT_STR_EQ("a", "b");
    EXPECT_STR_NE("a", "a");
    EXPECT_STR_LT("b", "b");
    EXPECT_STR_LE("b", "a");
    EXPECT_STR_GT("b", "b");
    EXPECT_STR_GE("a", "b");

    EXPECT_STR_IC_EQ("A", "b");
    EXPECT_STR_IC_NE("A", "a");
    EXPECT_STR_IC_LT("A", "a");
    EXPECT_STR_IC_LE("B", "a");
    EXPECT_STR_IC_GT("B", "b");
    EXPECT_STR_IC_GE("a", "b");
}

TEST_SUITE(test_expect_suite)
{
    test_expect_passed,
    test_expect_failed,
    TEST_NULL
};


/**
 * test_assert_suite
 */
TEST_CASE(test_assert_passed)
{
    ASSERT_TRUE(true);
    ASSERT_FALSE(false);

    ASSERT_EQ(0, 0);
    ASSERT_NE(0, 1);
    ASSERT_LT(0, 1);
    ASSERT_LE(1, 1);
    ASSERT_GT(1, 0);
    ASSERT_GE(1, 1);

    ASSERT_STR_EQ("a", "a");
    ASSERT_STR_NE("a", "b");
    ASSERT_STR_LT("a", "b");
    ASSERT_STR_LE("b", "b");
    ASSERT_STR_GT("b", "a");
    ASSERT_STR_GE("b", "b");

    ASSERT_STR_IC_EQ("A", "a");
    ASSERT_STR_IC_NE("A", "b");
    ASSERT_STR_IC_LT("A", "b");
    ASSERT_STR_IC_LE("B", "b");
    ASSERT_STR_IC_GT("B", "a");
    ASSERT_STR_IC_GE("B", "b");
}

TEST_CASE(test_assert_true_failed)
{
    ASSERT_TRUE(false);
    abort();
}
TEST_CASE(test_assert_false_failed)
{
    ASSERT_FALSE(true);
    abort();
}

TEST_CASE(test_assert_eq_failed)
{
    ASSERT_EQ(0, 1);
    abort();
}
TEST_CASE(test_assert_ne_failed)
{
    ASSERT_NE(1, 1);
    abort();
}
TEST_CASE(test_assert_lt_failed)
{
    ASSERT_LT(1, 1);
    abort();
}
TEST_CASE(test_assert_le_failed)
{
    ASSERT_LE(1, 0);
    abort();
}
TEST_CASE(test_assert_gt_failed)
{
    ASSERT_GT(1, 1);
    abort();
}
TEST_CASE(test_assert_ge_failed)
{
    ASSERT_GE(0, 1);
    abort();
}

TEST_CASE(test_assert_str_eq_failed)
{
    ASSERT_STR_EQ("a", "b");
    abort();
}
TEST_CASE(test_assert_str_ne_failed)
{
    ASSERT_STR_NE("a", "a");
    abort();
}
TEST_CASE(test_assert_str_lt_failed)
{
    ASSERT_STR_LT("b", "b");
    abort();
}
TEST_CASE(test_assert_str_le_failed)
{
    ASSERT_STR_LE("b", "a");
    abort();
}
TEST_CASE(test_assert_str_gt_failed)
{
    ASSERT_STR_GT("b", "b");
    abort();
}
TEST_CASE(test_assert_str_ge_failed)
{
    ASSERT_STR_GE("a", "b");
    abort();
}

TEST_CASE(test_assert_str_ic_eq_failed)
{
    ASSERT_STR_IC_EQ("a", "B");
    abort();
}
TEST_CASE(test_assert_str_ic_ne_failed)
{
    ASSERT_STR_IC_NE("a", "A");
    abort();
}
TEST_CASE(test_assert_str_ic_lt_failed)
{
    ASSERT_STR_IC_LT("B", "b");
    abort();
}
TEST_CASE(test_assert_str_ic_le_failed)
{
    ASSERT_STR_IC_LE("b", "A");
    abort();
}
TEST_CASE(test_assert_str_ic_gt_failed)
{
    ASSERT_STR_IC_GT("B", "b");
    abort();
}
TEST_CASE(test_assert_str_ic_ge_failed)
{
    ASSERT_STR_IC_GE("A", "B");
    abort();
}

TEST_SUITE(test_assert_suite)
{
    test_assert_passed,
    test_assert_true_failed,
    test_assert_false_failed,
    test_assert_eq_failed,
    test_assert_ne_failed,
    test_assert_lt_failed,
    test_assert_le_failed,
    test_assert_gt_failed,
    test_assert_ge_failed,
    test_assert_str_eq_failed,
    test_assert_str_ne_failed,
    test_assert_str_lt_failed,
    test_assert_str_le_failed,
    test_assert_str_gt_failed,
    test_assert_str_ge_failed,
    test_assert_str_ic_eq_failed,
    test_assert_str_ic_ne_failed,
    test_assert_str_ic_lt_failed,
    test_assert_str_ic_le_failed,
    test_assert_str_ic_gt_failed,
    test_assert_str_ic_ge_failed,
    TEST_NULL
};


TEST_RUNNER(test_assertion)
{
    test_expect_suite,
    test_assert_suite,
    TEST_NULL
};
