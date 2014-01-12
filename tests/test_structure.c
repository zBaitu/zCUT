#include <zcut.h>

/**
 * test_passed_suite
 */
TEST_CASE(test_no_assertion)
{
}

TEST_CASE(test_one_assertion_passed)
{
    EXPECT_TRUE(true);
}

TEST_CASE(test_more_assertion_passed)
{
    EXPECT_FALSE(false);
    EXPECT_EQ(0, 0);
    EXPECT_STR_EQ("a", "a");
}

TEST_SUITE(test_passed_suite)
{
    test_no_assertion,
    test_one_assertion_passed,
    test_more_assertion_passed,
    TEST_NULL
};


/**
 * test_failed_suite
 */
TEST_CASE(test_one_assertion_failed)
{
    EXPECT_TRUE(false);
}

TEST_CASE(test_more_assertion_failed)
{
    EXPECT_FALSE(true);
    EXPECT_EQ(0, 1);
    EXPECT_STR_EQ("a", "b");
}

TEST_SUITE(test_failed_suite)
{
    test_no_assertion,
    test_one_assertion_failed,
    test_more_assertion_failed,
    TEST_NULL
};


/**
 * test_suite_setup_teardown
 */
SUITE_SETUP(test_suite_setup_teardown)
{
    return true;
}

SUITE_TEARDOWN(test_suite_setup_teardown)
{
    return true;
}

TEST_SUITE(test_suite_setup_teardown)
{
    test_no_assertion,
    TEST_NULL
};


/**
 * test_suite_case_setup_teardown
 */
SUITE_SETUP(test_suite_case_setup_teardown)
{
    return true;
}

SUITE_TEARDOWN(test_suite_case_setup_teardown)
{
    return true;
}

CASE_SETUP(test_suite_case_setup_teardown)
{
    return true;
}

CASE_TEARDOWN(test_suite_case_setup_teardown)
{
    return true;
}

TEST_SUITE(test_suite_case_setup_teardown)
{
    test_no_assertion,
    test_no_assertion,
    TEST_NULL
};


/**
 * test_suite_setup_failed
 */
SUITE_SETUP(test_suite_setup_failed)
{
    return false;
}

TEST_SUITE(test_suite_setup_failed)
{
    test_no_assertion,
    TEST_NULL
};


RUNNER_SETUP()
{
    return true;
}

RUNNER_TEARDOWN()
{
    return true;
}

TEST_RUNNER(test_structure)
{
    test_passed_suite,
    test_failed_suite,
    test_suite_setup_teardown,
    test_suite_case_setup_teardown,
    test_suite_setup_failed,
    TEST_NULL
};
