#include <zcut.h>

TEST_CASE(test_no_assertion)
{
}

TEST_SUITE(test_no_assertion_suite)
{
    test_no_assertion,
    TEST_NULL
};

TEST_RUNNER(test_link_zcut)
{
    test_no_assertion_suite,
    TEST_NULL
};


int main(int argc, char* argv[])
{
    if (!ut_init(argc, argv))
      return EXIT_FAILURE;

    bool is_passed = ut_run();
    ut_fini();

    return (is_passed) ? EXIT_SUCCESS : EXIT_FAILURE;
}
