zCUT ---- C Unit Test Framework
===============================

https://github.com/zBaitu/zCUT


Make, Install, Use
------------------

### make and install
```
mkdir build
cd build
cmake ..
make
make install
```

### use
```
gcc ut.c -lzcut_main
gcc ut_main.c  -lzcut
```
There are 2 static lib, **libzcut.a** and **libzcut_main.a**.  
libzcut_main.a contains default main implementation, which in lib/zcut_main.c.


## Test Structure
There are 3 macro:  
TEST_CASE  
TEST_SUITE  
TEST_RUNNER  
```
#include <zcut.h>

TEST_CASE(test_case_name0)
{
    ...     /* Test body, assertion are here. */
}
TEST_CASE(test_case_name1)
{
    ...
}

TEST_SUITE(test_suite_name0)
{
    test_case_name0,
    test_case_name1,
    TEST_NULL
};

TEST_RUNNER(test_runner_name)
{
    test_suite_name0,
    TEST_NULL
};
```
Test case register in test suite, and test suite register in test runner.  
**DO NOT FORGET `TEST_NULL`** in TEST_SUITE and TEST_RUNNER.  
**DO NOT FORGET `;`** after TEST_SUITE and TEST_RUNNER.


# Assertion
Assertion prototype is:  
**[EXPECT|ASSERT]\_[TYPE]\_COMPARE(actual[, expected, user_msg...])**
```
/* boolean assertion */
EXPECT_TRUE
EXPECT_FALSE

/* number assertion */
EXPECT_EQ   /* equal */
EXPECT_NE   /* unequal */
EXPECT_LT   /* less than */
EXPECT_LE   /* less or equal */
EXPECT_GT   /* greater than */
EXPECT_GE   /* greater or equal */

/* string assertion */
EXPECT_STR_EQ
EXPECT_STR_NE
EXPECT_STR_LT
EXPECT_STR_LE
EXPECT_STR_GT
EXPECT_STR_GE

/* string assertion, ignore case */
EXPECT_STR_IC_EQ
EXPECT_STR_IC_NE
EXPECT_STR_IC_LT
EXPECT_STR_IC_LE
EXPECT_STR_IC_GT
EXPECT_STR_IC_GE
```
The difference between EXPECT and ASSERT is: When ASSERT_XXX failed, it will exit test case directly,
code after ASSERT will not execute.

The user_msg parameter in assertion is like format in `printf`.
You can add more test msg, it will print when case failed. For example:  
`EXPECT_EQ(0, 1, "error here:%d, %d", 0, 1);`


## Execution
There are some command options:
```
Usage: test_bin [-bC] [-fF FILTER_EXPRESSION] [-Hkl] [-r REPEAT_COUNT] [-s] [-x [XML_PATH]] [-hv]
  -b, --break-on-failure           Exit unit test when a assertion failed.
  -C, --no-color                   Disabled colored output. Default is enabled.
  -f, --case-filter                Choose test case to run with simple regular expression.
  -F, --suite-filter               Choose test suite to run with simple regular expression.
  -H, --highlight                  Enable highlighted output. Default is disabled.
  -k, --keep-going                 When repeat count is larger than 0, keep unit test going when error occur.
  -l, --list                       Just list out all the test suite and test case instead of running them.
  -r, --repeat                     Run unit test for a repeat count, in range [0, INT_MAX].
  -R, --no-filtered-out-result     Do not output filterd out case or suite result.
  -s, --shuffle                    Randomize the order of test suite and test case.
  -x, --xml-path                   Generate an XML report with detail informaion of the unit test.
  -h, --help                       Display this help and exit.
  -v, --version                    Display version and exit.
```
There are env parameters corresponding to command options:
```
-b  UT_BREAK_ON_FAILURE
-C  UT_NO_COLOR
-f  UT_CASE_FILTER
-F  UT_SUITE_FILTER
-H  UT_HIGHLIGHT
-k  UT_KEEP_GOING
-l  UT_LIST
-r  UT_REPEAT
-R  UT_NO_FILTERED_OUT_RESULT
-s  UT_SHUFFLE
-x  UT_XML_PATH
```


## MISC
For more detail, please see tests/test_XXX.c for demo.
