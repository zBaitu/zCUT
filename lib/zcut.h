#ifndef _ZCUT_H_
#define _ZCUT_H_

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

typedef enum const_t
{
    MAX_STR_LEN = 1024
}const_t;

typedef enum color_t
{
    RED     = 1,
    GREEN   = 2,
    YELLOW  = 3,
    BLUE    = 4,
    PURPLE  = 5,
    CYAN    = 6
}color_t;

typedef enum test_type_t
{
    RUNNER,
    SUITE,
    CASE,
    SETUP,
    TEARDOWN,
}test_type_t;

typedef struct case_result_t
{
    bool        accessed;
    bool        is_filtered_out;
    bool        passed;
    int         assertion_count;
    int         succ_assertion_count;
    int         fail_assertion_count;
    int         time;
    const char* file;
    int         line;
    char        expected[MAX_STR_LEN];
    char        actual[MAX_STR_LEN];
    char        user_msg[MAX_STR_LEN];
}case_result_t;

typedef void (*test_body_t)(struct case_result_t *result);
typedef struct test_case_t
{
    const char*     name;
    test_body_t     test;
    case_result_t   *result;
}test_case_t;

typedef struct suite_result_t
{
    bool    accessed;
    bool    is_filtered_out;
    bool    passed;
    int     case_count;
    int     succ_case_count;
    int     fail_case_count;
    int     skip_case_count;
    int     filtered_out_case_count;
    int     assertion_count;
    int     succ_assertion_count;
    int     fail_assertion_count;
    int     time;
}suite_result_t;

typedef bool (*setup_teardown_func_t)(void);
typedef test_case_t* (*get_case_func_t)(void);
typedef struct test_suite_t
{
    const char*             name;
    setup_teardown_func_t   *case_setup;
    setup_teardown_func_t   *case_teardown;
    setup_teardown_func_t   *suite_setup;
    setup_teardown_func_t   *suite_teardown;
    get_case_func_t         *get_case_func_list;
    int                     case_count;
    test_case_t*            *case_list;
    suite_result_t          *result;
}test_suite_t;

typedef struct runner_result_t
{
    bool    accessed;
    bool    passed;
    int     suite_count;
    int     succ_suite_count;
    char    (*succ_suite_list)[MAX_STR_LEN];
    int     fail_suite_count;
    char    (*fail_suite_list)[MAX_STR_LEN];
    int     skip_suite_count;
    char    (*skip_suite_list)[MAX_STR_LEN];
    int     filtered_out_suite_count;
    char    (*filtered_out_suite_list)[MAX_STR_LEN];
    int     case_count;
    int     succ_case_count;
    char    (*succ_case_list)[MAX_STR_LEN];
    int     fail_case_count;
    char    (*fail_case_list)[MAX_STR_LEN];
    int     skip_case_count;
    char    (*skip_case_list)[MAX_STR_LEN];
    int     filtered_out_case_count;
    char    (*filtered_out_case_list)[MAX_STR_LEN];
    int     assertion_count;
    int     succ_assertion_count;
    int     fail_assertion_count;
    int     time;
}runner_result_t;

typedef test_suite_t* (*get_suite_func_t)(void);
typedef struct test_runner_t
{
    const char*             test_bin_name;
    const char*             name;
    setup_teardown_func_t   *setup;
    setup_teardown_func_t   *teardown;
    get_suite_func_t        *get_suite_func_list;
    int                     suite_count;
    test_suite_t*           *suite_list;
    runner_result_t         *result;
}test_runner_t;


#define UT_FLAG(name) ut_flag_##name
extern bool UT_FLAG(break_on_failure);
extern char UT_FLAG(case_filter)[MAX_STR_LEN];
extern char UT_FLAG(suite_filter)[MAX_STR_LEN];
extern bool UT_FLAG(help);
extern bool UT_FLAG(keep_going);
extern bool UT_FLAG(list);
extern bool UT_FLAG(no_color);
extern bool UT_FLAG(no_filtered_out_result);
extern bool UT_FLAG(highlight);
extern int  UT_FLAG(repeat);
extern bool UT_FLAG(shuffle);
extern bool UT_FLAG(version);
extern bool UT_FLAG(xml);
extern char UT_FLAG(xml_path)[MAX_STR_LEN];


#define TEST_NULL   NULL
#define EMPTY_STR   ""
#define RETURN      true
#define NO_RETURN   false

#define ATTRIBUTE_UNUSED __attribute__((unused))

#define TEST_CASE(case_name)\
    void case_name##_test_body(case_result_t *CASE_RESULT_PARAMETER);\
    case_result_t case_name##_case_result;\
    test_case_t case_name##_test_case =\
    {\
        #case_name,\
        case_name##_test_body,\
        &case_name##_case_result\
    };\
    test_case_t* case_name(void)\
    {\
        return &case_name##_test_case;\
    }\
    void case_name##_test_body(case_result_t *CASE_RESULT_PARAMETER ATTRIBUTE_UNUSED)

#define CASE_SETUP(suite_name)\
    bool suite_name##_case_setup(void);\
    setup_teardown_func_t suite_name##_case_setup_func = suite_name##_case_setup;\
    bool suite_name##_case_setup(void)

#define CASE_TEARDOWN(suite_name)\
    bool suite_name##_case_teardown(void);\
    setup_teardown_func_t suite_name##_case_teardown_func = suite_name##_case_teardown;\
    bool suite_name##_case_teardown(void)

#define SUITE_SETUP(suite_name)\
    bool suite_name##_suite_setup(void);\
    setup_teardown_func_t suite_name##_suite_setup_func = suite_name##_suite_setup;\
    bool suite_name##_suite_setup(void)

#define SUITE_TEARDOWN(suite_name)\
    bool suite_name##_suite_teardown(void);\
    setup_teardown_func_t suite_name##_suite_teardown_func = suite_name##_suite_teardown;\
    bool suite_name##_suite_teardown(void)

#define TEST_SUITE(suite_name)\
    setup_teardown_func_t suite_name##_case_setup_func;\
    setup_teardown_func_t suite_name##_case_teardown_func;\
    setup_teardown_func_t suite_name##_suite_setup_func;\
    setup_teardown_func_t suite_name##_suite_teardown_func;\
    get_case_func_t suite_name##_case_list[];\
    suite_result_t suite_name##_suite_result;\
    test_suite_t suite_name##_test_suite =\
    {\
        #suite_name,\
        &suite_name##_case_setup_func,\
        &suite_name##_case_teardown_func,\
        &suite_name##_suite_setup_func,\
        &suite_name##_suite_teardown_func,\
        suite_name##_case_list,\
        0,\
        NULL,\
        &suite_name##_suite_result\
    };\
    test_suite_t* suite_name(void)\
    {\
        return &suite_name##_test_suite;\
    }\
    get_case_func_t suite_name##_case_list[]=

#define RUNNER_SETUP(void)\
    bool runner_setup(void);\
    setup_teardown_func_t runner_setup_func = runner_setup;\
    bool runner_setup(void)

#define RUNNER_TEARDOWN(void)\
    bool runner_teardown(void);\
    setup_teardown_func_t runner_teardown_func = runner_teardown;\
    bool runner_teardown(void)

#define TEST_RUNNER(runner_name)\
    setup_teardown_func_t runner_setup_func;\
    setup_teardown_func_t runner_teardown_func;\
    get_suite_func_t runner_suite_list[];\
    runner_result_t runner_name##_runner_result;\
    test_runner_t _test_runner_ =\
    {\
        NULL,\
        #runner_name,\
        &runner_setup_func,\
        &runner_teardown_func,\
        runner_suite_list,\
        0,\
        NULL,\
        &runner_name##_runner_result\
    };\
    get_suite_func_t runner_suite_list[]=


#define FORMAT_BOOL(actual, compare, expected)\
    snprintf(expected_str, MAX_STR_LEN, "(%s) %s %s", #actual, #compare, (expected ? "true" : "false"));\
    snprintf(actual_str, MAX_STR_LEN, "(%s) == %s", #actual, (actual ? "true" : "false"))

#define FORMAT_STR(actual, compare, expected)\
    snprintf(expected_str, MAX_STR_LEN, "%s %s \"%s\"", #actual, #compare, expected);\
    snprintf(actual_str, MAX_STR_LEN, "%s == \"%s\"", #actual, actual)

#define FORMAT_STR_IC(actual, compare, expected)\
    snprintf(expected_str, MAX_STR_LEN, "%s %s \"%s\" (ignore case)", #actual, #compare, expected);\
    snprintf(actual_str, MAX_STR_LEN, "%s == \"%s\" (ignore case)", #actual, actual)

#define FORMAT_INT(actual, compare, expected)\
    if (actual >= 0)\
    {\
        if (sizeof(actual) > 4)\
        {\
            snprintf(expected_str, MAX_STR_LEN, "%s %s %llu", #actual, #compare, (unsigned long long)expected);\
            snprintf(actual_str, MAX_STR_LEN, "%s == %llu", #actual, (unsigned long long)actual);\
        }\
        else\
        {\
            snprintf(expected_str, MAX_STR_LEN, "%s %s %lu", #actual, #compare, (unsigned long)expected);\
            snprintf(actual_str, MAX_STR_LEN, "%s == %lu", #actual, (unsigned long)actual);\
        }\
    }\
    else\
    {\
        if (sizeof(actual) > 4)\
        {\
            snprintf(expected_str, MAX_STR_LEN, "%s %s %llu", #actual, #compare, (long long)expected);\
            snprintf(actual_str, MAX_STR_LEN, "%s == %lld", #actual, (long long)actual);\
        }\
        else\
        {\
            snprintf(expected_str, MAX_STR_LEN, "%s %s %lu", #actual, #compare, (long)expected);\
            snprintf(actual_str, MAX_STR_LEN, "%s == %ld", #actual, (long)actual);\
        }\
    }

#define ASSERTION(is_return, condition, actual, compare, expected, format, msg...)\
    if ((condition))\
    {\
        CASE_RESULT_PARAMETER->succ_assertion_count++;\
    }\
    else\
    {\
        CASE_RESULT_PARAMETER->fail_assertion_count++;\
        char expected_str[MAX_STR_LEN];\
        char actual_str[MAX_STR_LEN];\
        format(actual, compare, expected);\
        print_assertion_info(__FILE__, __LINE__, expected_str, actual_str, EMPTY_STR msg);\
        if (UT_FLAG(break_on_failure))\
            abort();\
        save_assertion_info(CASE_RESULT_PARAMETER, __FILE__, __LINE__, expected_str, actual_str, EMPTY_STR msg);\
        if (is_return)\
            return;\
    }

#define TEST_TRUE(is_return, expr, msg...)\
    ASSERTION(is_return, expr, expr, ==, true, FORMAT_BOOL, msg)
#define TEST_FALSE(is_return, expr, expr_str, msg...)\
    ASSERTION(is_return, !(expr), expr, ==, false, FORMAT_BOOL, msg)
#define EXPECT_TRUE(expr, msg...)   TEST_TRUE(NO_RETURN, expr, msg)
#define EXPECT_FALSE(expr, msg...)  TEST_FALSE(NO_RETURN, expr, msg)
#define ASSERT_TRUE(expr, msg...)   TEST_TRUE(RETURN, expr, msg)
#define ASSERT_FALSE(expr, msg...)  TEST_FALSE(RETURN, expr, msg)

#define TEST_NUM(is_return, actual, compare, expected, msg...)\
    ASSERTION(is_return, actual compare expected, actual, compare, expected, FORMAT_INT, msg)
#define EXPECT_EQ(actual, expected, msg...) TEST_NUM(NO_RETURN, actual, ==, expected, msg)
#define EXPECT_NE(actual, expected, msg...) TEST_NUM(NO_RETURN, actual, !=, expected, msg)
#define EXPECT_LT(actual, expected, msg...) TEST_NUM(NO_RETURN, actual, <, expected, msg)
#define EXPECT_LE(actual, expected, msg...) TEST_NUM(NO_RETURN, actual, <=, expected, msg)
#define EXPECT_GT(actual, expected, msg...) TEST_NUM(NO_RETURN, actual, >, expected, msg)
#define EXPECT_GE(actual, expected, msg...) TEST_NUM(NO_RETURN, actual, >=, expected, msg)
#define ASSERT_EQ(actual, expected, msg...) TEST_NUM(RETURN, actual, ==, expected, msg)
#define ASSERT_NE(actual, expected, msg...) TEST_NUM(RETURN, actual, !=, expected, msg)
#define ASSERT_LT(actual, expected, msg...) TEST_NUM(RETURN, actual, <, expected, msg)
#define ASSERT_LE(actual, expected, msg...) TEST_NUM(RETURN, actual, <=, expected, msg)
#define ASSERT_GT(actual, expected, msg...) TEST_NUM(RETURN, actual, >, expected, msg)
#define ASSERT_GE(actual, expected, msg...) TEST_NUM(RETURN, actual, >=, expected, msg)

#define TEST_STR(is_return, actual, compare, expected, msg...)\
    ASSERTION(is_return, strcmp(actual, expected) compare 0, actual, compare, expected, FORMAT_STR, msg)
#define EXPECT_STR_EQ(actual, expected, msg...) TEST_STR(NO_RETURN, actual, ==, expected, msg)
#define EXPECT_STR_NE(actual, expected, msg...) TEST_STR(NO_RETURN, actual, !=, expected, msg)
#define EXPECT_STR_LT(actual, expected, msg...) TEST_STR(NO_RETURN, actual, <, expected, msg)
#define EXPECT_STR_LE(actual, expected, msg...) TEST_STR(NO_RETURN, actual, <=, expected, msg)
#define EXPECT_STR_GT(actual, expected, msg...) TEST_STR(NO_RETURN, actual, >, expected, msg)
#define EXPECT_STR_GE(actual, expected, msg...) TEST_STR(NO_RETURN, actual, >=, expected, msg)
#define ASSERT_STR_EQ(actual, expected, msg...) TEST_STR(RETURN, actual, ==, expected, msg)
#define ASSERT_STR_NE(actual, expected, msg...) TEST_STR(RETURN, actual, !=, expected, msg)
#define ASSERT_STR_LT(actual, expected, msg...) TEST_STR(RETURN, actual, <, expected, msg)
#define ASSERT_STR_LE(actual, expected, msg...) TEST_STR(RETURN, actual, <=, expected, msg)
#define ASSERT_STR_GT(actual, expected, msg...) TEST_STR(RETURN, actual, >, expected, msg)
#define ASSERT_STR_GE(actual, expected, msg...) TEST_STR(RETURN, actual, >=, expected, msg)

#define TEST_STR_IC(is_return, actual, compare, expected, msg...)\
    ASSERTION(is_return, strcasecmp(actual, expected) compare 0, actual, compare, expected, FORMAT_STR_IC, msg)
#define EXPECT_STR_IC_EQ(actual, expected, msg...) TEST_STR_IC(NO_RETURN, actual, ==, expected, msg)
#define EXPECT_STR_IC_NE(actual, expected, msg...) TEST_STR_IC(NO_RETURN, actual, !=, expected, msg)
#define EXPECT_STR_IC_LT(actual, expected, msg...) TEST_STR_IC(NO_RETURN, actual, <, expected, msg)
#define EXPECT_STR_IC_LE(actual, expected, msg...) TEST_STR_IC(NO_RETURN, actual, <=, expected, msg)
#define EXPECT_STR_IC_GT(actual, expected, msg...) TEST_STR_IC(NO_RETURN, actual, >, expected, msg)
#define EXPECT_STR_IC_GE(actual, expected, msg...) TEST_STR_IC(NO_RETURN, actual, >=, expected, msg)
#define ASSERT_STR_IC_EQ(actual, expected, msg...) TEST_STR_IC(RETURN, actual, ==, expected, msg)
#define ASSERT_STR_IC_NE(actual, expected, msg...) TEST_STR_IC(RETURN, actual, !=, expected, msg)
#define ASSERT_STR_IC_LT(actual, expected, msg...) TEST_STR_IC(RETURN, actual, <, expected, msg)
#define ASSERT_STR_IC_LE(actual, expected, msg...) TEST_STR_IC(RETURN, actual, <=, expected, msg)
#define ASSERT_STR_IC_GT(actual, expected, msg...) TEST_STR_IC(RETURN, actual, >, expected, msg)
#define ASSERT_STR_IC_GE(actual, expected, msg...) TEST_STR_IC(RETURN, actual, >=, expected, msg)


void save_assertion_info(case_result_t *result, const char* file, int line, const char* expected, const char* actual,
                         const char* msg, ...);
bool ut_init(int argc, char* argv[]);
bool ut_run(void);
void ut_fini(void);

void print_help(void);
void print_version(void);
void print_runner_begin(const test_runner_t *test_runner);
void print_runner_end(const test_runner_t *test_runner, setup_teardown_func_t teardown);
void print_suite_begin(const test_suite_t *test_suite);
void print_suite_end(const test_suite_t *test_suite);
void print_case_begin(const test_case_t *test_case);
void print_case_end(const test_case_t *test_case);
void print_setup_begin(test_type_t test_type);
void print_setup_end(test_type_t test_type, bool passed, int time);
void print_teardown_begin(test_type_t test_type);
void print_teardown_end(test_type_t test_type, bool passed, int time);
void print_assertion_info(const char* file, int line, const char* expected, const char* actual, const char* msg, ...);
void print_ut_list(const test_runner_t *test_runner);
void print_ut_result(const test_runner_t *test_runner);
bool print_ut_result_to_xml(const test_runner_t *test_runner, int repeat);

void print_ut_init_no_called_error(void);
void print_ut_init_error(void);
void print_ut_flag_int_type_warning(const char* flag, const char* value, int default_value);
void print_ut_flag_int_value_warning(const char* flag, int value, int min, int max, int default_value);
void print_ut_flag_int_type_error(const char* option, const char* value);
void print_ut_flag_int_value_error(const char* option, int value, int min, int max);
void print_non_option_error(int optind, int argc, char* argv[]);
void print_error(const char* file, const char* function, int line, const char* msg, ...);

#endif
