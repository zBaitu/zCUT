#include <zcut.h>

/**
 * test_sample_suite
 */
TEST_CASE(test_passed)
{
    EXPECT_TRUE(true);
}

TEST_CASE(test_failed)
{
    int num = 0;
    EXPECT_EQ(num, 1);
}

TEST_SUITE(test_sample_suite)
{
    test_passed,
    test_failed,
    TEST_NULL
};

TEST_RUNNER(test_sample)
{
    test_sample_suite,
    TEST_NULL
};
