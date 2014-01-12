#include "zcut.h"

#include <getopt.h>
#include <libgen.h>
#include <stdarg.h>
#include <time.h>

#define PRINT_INTERNAL_ERROR(msg...) print_error(__FILE__, __func__, __LINE__, EMPTY_STR msg)
#define GET_TIME_OF_DAY(now)\
    struct timeval now;\
    gettimeofday(&now, NULL)
#define GET_TIME_PAST(begin, end) ((end.tv_sec - begin.tv_sec) * 1000 + (end.tv_usec - begin.tv_usec) / 1000)

extern test_runner_t _test_runner_;

bool UT_FLAG(break_on_failure);
char UT_FLAG(case_filter)[MAX_STR_LEN];
char UT_FLAG(suite_filter)[MAX_STR_LEN];
bool UT_FLAG(help);
bool UT_FLAG(keep_going);
bool UT_FLAG(list);
int  UT_FLAG(repeat) = 1;
bool UT_FLAG(shuffle);
bool UT_FLAG(version);
bool UT_FLAG(xml);

static bool _is_ut_init_called_;
static bool _is_ut_init_successed_;

static bool get_env_bool(const char* key, bool *value)
{
    *value = (getenv(key) != NULL);
    return *value;
}

static bool get_env_int(const char* key, int *value)
{
    const char* value_str = getenv(key);
    if (value_str == NULL)
        return false;

    char* endptr = NULL;
    int env_value = strtol(value_str, &endptr, 10);
    if (*endptr != '\0')
    {
        print_ut_flag_int_type_warning(key, value_str, *value);
        return false;
    }

    if (env_value < 0)
    {
        print_ut_flag_int_value_warning(key, env_value, 0, INT_MAX, *value);
        return false;
    }

    *value = env_value;
    return true;
}

static bool get_env_str(const char* key, char value[MAX_STR_LEN])
{
    const char* env_value = getenv(key);
    if (env_value == NULL)
        return false;

    snprintf(value, MAX_STR_LEN, "%s", env_value);
    return true;
}

static void get_ut_flag_from_env_var(void)
{
    get_env_bool("UT_BREAK_ON_FAILURE", &UT_FLAG(break_on_failure));
    get_env_str("UT_CASE_FILTER", UT_FLAG(case_filter));
    get_env_str("UT_SUITE_FILTER", UT_FLAG(suite_filter));
    get_env_bool("UT_KEEP_GOING", &UT_FLAG(keep_going));
    get_env_bool("UT_LIST", &UT_FLAG(list));
    get_env_bool("UT_NO_COLOR", &UT_FLAG(no_color));
    get_env_bool("UT_NO_FILTERED_OUT_RESULT", &UT_FLAG(no_filtered_out_result));
    get_env_bool("UT_HIGHLIGHT", &UT_FLAG(highlight));
    get_env_int("UT_REPEAT", &UT_FLAG(repeat));
    get_env_bool("UT_SHUFFLE", &UT_FLAG(shuffle));

    if (get_env_str("UT_XML_PATH", UT_FLAG(xml_path)))
        UT_FLAG(xml) = true;
}

static bool get_ut_flag_from_cmd_line(int argc, char* argv[])
{
    char* short_options = "bCf:F:Hklr:Rsx::hv";
    struct option long_options[] =
    {
        {"break-on-failure",        no_argument,        0, 'b'},
        {"no-color",                no_argument,        0, 'C'},
        {"case-filter",             required_argument,  0, 'f'},
        {"suite-filter",            required_argument,  0, 'F'},
        {"highlight",               no_argument,        0, 'H'},
        {"keep-going",              no_argument,        0, 'k'},
        {"list",                    no_argument,        0, 'l'},
        {"repeat",                  required_argument,  0, 'r'},
        {"no-filtered-out-result",  no_argument,        0, 'R'},
        {"shuffle",                 no_argument,        0, 's'},
        {"xml-path",                optional_argument,  0, 'x'},
        {"help",                    no_argument,        0, 'h'},
        {"version",                 no_argument,        0, 'v'},
        {0, 0, 0, 0}
    };
    int option_index = 0;
    int option;

    while ((option = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        char* endptr = NULL;
        int repeat;
        const char* cur_option;

        switch (option)
        {
        case 'b':
            UT_FLAG(break_on_failure) = true;
            break;
        case 'C':
            UT_FLAG(no_color) = true;
            break;
        case 'f':
            snprintf(UT_FLAG(case_filter), sizeof(UT_FLAG(case_filter)), "%s", optarg);
            break;
        case 'F':
            snprintf(UT_FLAG(suite_filter), sizeof(UT_FLAG(suite_filter)), "%s", optarg);
            break;
        case 'H':
            UT_FLAG(highlight) = true;
            break;
        case 'k':
            UT_FLAG(keep_going) = true;
            break;
        case 'l':
            UT_FLAG(list) = true;
            break;
        case 'r':
            repeat = strtol(optarg, &endptr, 10);
            if (*endptr != '\0' || repeat < 0)
            {
                if (option_index != 0)
                    cur_option = long_options[option_index].name;
                else
                    cur_option = "-r";

                if (*endptr != '\0')
                    print_ut_flag_int_type_error(cur_option, optarg);
                else
                    print_ut_flag_int_value_error(cur_option, repeat, 0, INT_MAX);

                print_help();
                return false;
            }

            UT_FLAG(repeat) = repeat;
            break;
        case 'R':
            UT_FLAG(no_filtered_out_result) = true;
            break;
        case 's':
            UT_FLAG(shuffle) = true;
            break;
        case 'x':
            UT_FLAG(xml) = true;
            if (optarg != NULL)
                snprintf(UT_FLAG(xml_path), sizeof(UT_FLAG(xml_path)), "%s", optarg);
            break;
        case 'h':
            UT_FLAG(help) = true;
            break;
        case 'v':
            UT_FLAG(version) = true;
            break;
        default:
            print_help();
            return false;
        }
    }

    if (optind < argc)
    {
        print_non_option_error(optind, argc, argv);
        return false;
    }

    return true;
}

static bool init_ut_flag(int argc, char* argv[])
{
    get_ut_flag_from_env_var();
    return get_ut_flag_from_cmd_line(argc, argv);
}

static int get_runner_suite_count(const test_runner_t *test_runner)
{
    int count = 0;
    int i = 0;
    get_suite_func_t get_suite_case_func;
    while ((get_suite_case_func = test_runner->get_suite_func_list[i++]) != TEST_NULL)
        count++;

    return count;
}

static int get_suite_case_count(const test_suite_t *test_suite)
{
    int case_count = 0;
    while (test_suite->get_case_func_list[case_count] != TEST_NULL)
        case_count++;

    return case_count;
}

static bool alloc_suite_case_list(test_suite_t *test_suite)
{
    if (test_suite->case_list != NULL)
        return true;

    int case_count = get_suite_case_count(test_suite);
    test_suite->case_count = case_count;
    test_suite->case_list = (test_case_t**)malloc(case_count * sizeof(test_case_t*));
    if (test_suite->case_list == NULL)
    {
        PRINT_INTERNAL_ERROR("malloc(%d): %m", case_count * sizeof(test_case_t*));
        return false;
    }

    return true;
}

static bool alloc_runner_suite_list(test_runner_t *test_runner)
{
    if (test_runner->suite_list != NULL)
        return true;

    int suite_count = get_runner_suite_count(test_runner);
    test_runner->suite_count = suite_count;
    test_runner->suite_list = (test_suite_t**)malloc(suite_count * sizeof(test_suite_t*));
    if (test_runner->suite_list == NULL)
    {
        PRINT_INTERNAL_ERROR("malloc(%d): %m", suite_count * sizeof(test_suite_t*));
        return false;
    }

    return true;
}

static bool init_suite_case_list(test_suite_t *test_suite)
{
    if (!alloc_suite_case_list(test_suite))
        return false;

    int case_index = 0;
    int i = 0;
    get_case_func_t get_case_func;
    while ((get_case_func = test_suite->get_case_func_list[i++]) != TEST_NULL)
    {
        test_case_t *test_case = (test_case_t*)((*get_case_func)());
        test_suite->case_list[case_index++] = test_case;
    }

    return true;
}

static bool init_runner_suite_list(test_runner_t *test_runner)
{
    if (!alloc_runner_suite_list(test_runner))
        return false;

    int suite_index = 0;
    int i = 0;
    get_suite_func_t get_suite_case_func;
    while ((get_suite_case_func = test_runner->get_suite_func_list[i++]) != TEST_NULL)
    {
        void* suite_case = (*get_suite_case_func)();
        test_suite_t *test_suite = (test_suite_t*)(suite_case);
        if (!init_suite_case_list(test_suite))
            return false;

        test_runner->suite_list[suite_index++] = test_suite;
    }

    return true;
}

static void shuffle_list(void** list, int len)
{
    srand(time(NULL));

    int i;
    for (i = len - 1; i > 0; i--)
    {
        int selected = (int)((i + 1) * (rand() / (RAND_MAX + 1.0)));
        void* tmp = list[i];
        list[i] = list[selected];
        list[selected] = tmp;
    }
}

static void shuffle(test_runner_t *test_runner)
{
    shuffle_list((void**)(test_runner->suite_list), test_runner->suite_count);

    int i;
    for (i = 0; i < test_runner->suite_count; i++)
        shuffle_list((void**)(test_runner->suite_list[i]->case_list), test_runner->suite_list[i]->case_count);
}

static bool alloc_runner_result_list(char (**list)[MAX_STR_LEN], int count)
{
    if (*list != NULL)
        return true;

    *list = (char (*)[MAX_STR_LEN])malloc(count * sizeof(char[MAX_STR_LEN]));
    if (*list == NULL)
    {
        PRINT_INTERNAL_ERROR("malloc(%d): %m", count * sizeof(char[MAX_STR_LEN]));
        return false;
    }

    return true;
}

static bool alloc_runner_result_suite_list(test_runner_t *test_runner)
{
    int suite_count = test_runner->suite_count;
    runner_result_t *result = test_runner->result;

    if (!alloc_runner_result_list(&result->succ_suite_list, suite_count))
        return false;
    if (!alloc_runner_result_list(&result->fail_suite_list, suite_count))
        return false;
    if (!alloc_runner_result_list(&result->skip_suite_list, suite_count))
        return false;
    if (!alloc_runner_result_list(&result->filtered_out_suite_list, suite_count))
        return false;

    return true;
}

static int get_total_case_count(const test_runner_t *test_runner)
{
    int total_count = 0;
    int i;
    for (i = 0; i < test_runner->suite_count; i++)
        total_count += test_runner->suite_list[i]->case_count;

    return total_count;
}

static bool alloc_runner_result_case_list(test_runner_t *test_runner)
{
    int case_count = get_total_case_count(test_runner);
    runner_result_t *result = test_runner->result;

    if (!alloc_runner_result_list(&result->succ_case_list, case_count))
        return false;
    if (!alloc_runner_result_list(&result->fail_case_list, case_count))
        return false;
    if (!alloc_runner_result_list(&result->skip_case_list, case_count))
        return false;
    if (!alloc_runner_result_list(&result->filtered_out_case_list, case_count))
        return false;

    return true;
}

static bool init_test_runner(const char* test_bin_path, test_runner_t *test_runner)
{
    if (!init_runner_suite_list(test_runner))
        return false;

    if (UT_FLAG(shuffle))
        shuffle(test_runner);

    if (!alloc_runner_result_suite_list(test_runner))
        return false;

    if (!alloc_runner_result_case_list(test_runner))
        return false;

    test_runner->test_bin_name = basename((char*)test_bin_path);
    return true;
}

static bool should_run(void)
{
    if (UT_FLAG(help))
    {
        print_help();
        return false;
    }

    if (UT_FLAG(version))
    {
        print_version();
        return false;
    }

    if (UT_FLAG(list))
    {
        print_ut_list(&_test_runner_);
        return false;
    }

    return true;
}

static bool run_setup_teardown(test_type_t test_type, test_type_t setup_teardown,
                               setup_teardown_func_t setup_teardown_func)
{
    if (setup_teardown_func == 0)
        return true;

    switch (setup_teardown)
    {
    case SETUP:
        print_setup_begin(test_type);
        break;
    case TEARDOWN:
        print_teardown_begin(test_type);
        break;
    default:
        PRINT_INTERNAL_ERROR("error test_type_t `%d'\n", setup_teardown);
        abort();
    }

    GET_TIME_OF_DAY(begin);
    bool ret = (*setup_teardown_func)();
    GET_TIME_OF_DAY(end);
    int time = GET_TIME_PAST(begin, end);

    switch (setup_teardown)
    {
    case SETUP:
        print_setup_end(test_type, ret, time);
        break;
    case TEARDOWN:
        print_teardown_end(test_type, ret, time);
        break;
    default:
        PRINT_INTERNAL_ERROR("error test_type_t `%d'\n", setup_teardown);
        abort();
    }

    return ret;
}

static bool run_setup(test_type_t test_type, setup_teardown_func_t setup)
{
    return run_setup_teardown(test_type, SETUP, setup);
}

static bool run_teardown(test_type_t test_type, setup_teardown_func_t teardown)
{
    return run_setup_teardown(test_type, TEARDOWN, teardown);
}

static bool match_filter(const char* name, const char* fileter)
{
    switch (*fileter)
    {
    case '\0':
        return *name == '\0';
    case '?':
        return *name != '\0' && match_filter(name + 1, fileter + 1);
    case '*':
        return (*name != '\0' && match_filter(name + 1, fileter)) || match_filter(name, fileter + 1);
    default:
        return *name == *fileter && match_filter(name + 1, fileter + 1);
    }
}

static bool is_filtered_out(const char* name, test_type_t test_type)
{
    const char* filter;
    switch (test_type)
    {
    case CASE:
        filter = UT_FLAG(case_filter);
        break;
    case SUITE:
        filter = UT_FLAG(suite_filter);
        break;
    default:
        PRINT_INTERNAL_ERROR("error test_type_t `%d'\n", test_type);
        abort();
    }

    if (*filter == '\0')
        return false;

    if (match_filter(name, filter))
        return false;

    return true;
}

static void clear_case_result(case_result_t *result)
{
    memset(result, 0, sizeof(*result));
}

static void run_test_case(const test_case_t *test_case)
{
    case_result_t *result = test_case->result;
    clear_case_result(result);
    result->accessed = true;
    if (is_filtered_out(test_case->name, CASE))
    {
        result->is_filtered_out = true;
        return;
    }

    print_case_begin(test_case);

    GET_TIME_OF_DAY(begin);
    test_case->test(result);
    GET_TIME_OF_DAY(end);
    result->time = GET_TIME_PAST(begin, end);
    result->passed = result->fail_assertion_count > 0 ? false : true;
    result->assertion_count = result->succ_assertion_count + result->fail_assertion_count;

    print_case_end(test_case);
}

static void clear_suite_result(suite_result_t *result)
{
    memset(result, 0, sizeof(*result));
}

static void calc_suite_case_result(suite_result_t *suite_result, const case_result_t *case_result)
{
    if (case_result->is_filtered_out)
        return;

    suite_result->case_count++;

    if (!case_result->passed)
        suite_result->passed = false;

    suite_result->assertion_count += case_result->assertion_count;
    suite_result->time += case_result->time;
}

static void filter_out_suite_case(const test_suite_t *test_suite)
{
    test_suite->result->is_filtered_out = true;
    int i;
    for (i = 0; i < test_suite->case_count; i++)
    {
        test_suite->case_list[i]->result->accessed = true;
        test_suite->case_list[i]->result->is_filtered_out = true;
    }
}

static bool run_test_suite(const test_suite_t *test_suite)
{
    suite_result_t *result = test_suite->result;
    clear_suite_result(result);
    result->accessed = true;

    if (is_filtered_out(test_suite->name, SUITE))
    {
        filter_out_suite_case(test_suite);
        return true;
    }

    result->passed = true;
    print_suite_begin(test_suite);
    if (!run_setup(SUITE, *test_suite->suite_setup))
        goto RUN_SUITE_FAILED;

    int i;
    test_case_t** case_list = test_suite->case_list;
    for (i = 0; i < test_suite->case_count; i++)
    {
        if (!run_setup(CASE, *test_suite->case_setup))
            goto RUN_SUITE_FAILED;

        run_test_case(case_list[i]);
        calc_suite_case_result(result, case_list[i]->result);

        if (!run_teardown(CASE, *test_suite->case_teardown))
            goto RUN_SUITE_FAILED;
    }

    if (!run_teardown(SUITE, *test_suite->suite_teardown))
        goto RUN_SUITE_FAILED;
    print_suite_end(test_suite);
    return true;

RUN_SUITE_FAILED:
    result->passed = false;
    print_suite_end(test_suite);
    return false;
}

static void clear_runner_result(runner_result_t *result)
{
    char (*succ_suite_list)[MAX_STR_LEN] = result->succ_suite_list;
    char (*fail_suite_list)[MAX_STR_LEN] = result->fail_suite_list;
    char (*skip_suite_list)[MAX_STR_LEN] = result->skip_suite_list;
    char (*filtered_out_suite_list)[MAX_STR_LEN] = result->filtered_out_suite_list;
    char (*succ_case_list)[MAX_STR_LEN] = result->succ_case_list;
    char (*fail_case_list)[MAX_STR_LEN] = result->fail_case_list;
    char (*skip_case_list)[MAX_STR_LEN] = result->skip_case_list;
    char (*filtered_out_case_list)[MAX_STR_LEN] = result->filtered_out_case_list;

    memset(result, 0, sizeof(*result));

    result->succ_suite_list = succ_suite_list;
    result->fail_suite_list = fail_suite_list;
    result->skip_suite_list = skip_suite_list;
    result->filtered_out_suite_list = filtered_out_suite_list;
    result->succ_case_list = succ_case_list;
    result->fail_case_list = fail_case_list;
    result->skip_case_list = skip_case_list;
    result->filtered_out_case_list = filtered_out_case_list;
}

static void calc_runner_suite_result(runner_result_t *runner_result, const suite_result_t *suite_result)
{
    if (suite_result->is_filtered_out)
        return;

    runner_result->suite_count++;
    runner_result->case_count += suite_result->case_count;

    if (!suite_result->passed)
        runner_result->passed = false;

    runner_result->assertion_count += suite_result->assertion_count;
    runner_result->time += suite_result->time;
}

static bool run_test_runner(const test_runner_t *test_runner)
{
    runner_result_t *result = test_runner->result;
    clear_runner_result(result);
    result->accessed = true;
    result->passed = true;

    print_runner_begin(test_runner);
    if (!run_setup(RUNNER, *test_runner->setup))
        goto RUN_UT_FAILED;

    int i;
    test_suite_t** suite_list = test_runner->suite_list;
    for (i = 0; i < test_runner->suite_count; i++)
    {
        bool ret = run_test_suite(suite_list[i]);
        calc_runner_suite_result(result, suite_list[i]->result);
        if (!ret)
            goto RUN_UT_FAILED;
    }

    if (!run_teardown(RUNNER, *test_runner->teardown))
        goto RUN_UT_FAILED;
    print_runner_end(test_runner, *test_runner->teardown);
    return true;

RUN_UT_FAILED:
    print_runner_end(test_runner, *test_runner->teardown);
    return false;
}

static void reset_runner_result(runner_result_t *result)
{
    result->suite_count = 0;
    result->case_count = 0;
    result->assertion_count = 0;
    result->time = 0;
}

static void reset_suite_result(suite_result_t *result)
{
    result->case_count = 0;
    result->assertion_count = 0;
    result->time = 0;
}

static void calc_runner_suite_case_final_result(runner_result_t *runner_result, const test_suite_t *test_suite)
{
    suite_result_t *suite_result = test_suite->result;

    int i;
    test_case_t** case_list = test_suite->case_list;
    for (i = 0; i < test_suite->case_count; i++)
    {
        case_result_t *case_result = case_list[i]->result;
        if (!case_result->accessed)
        {
            suite_result->skip_case_count++;
            snprintf(runner_result->skip_case_list[runner_result->skip_case_count++], MAX_STR_LEN, "%s.%s",
                    test_suite->name, case_list[i]->name);
            continue;
        }
        if (case_result->is_filtered_out)
        {
            suite_result->filtered_out_case_count++;
            snprintf(runner_result->filtered_out_case_list[runner_result->filtered_out_case_count++],
                    MAX_STR_LEN, "%s.%s", test_suite->name, case_list[i]->name);
            continue;
        }
        if (case_result->passed)
        {
            suite_result->succ_case_count++;
            snprintf(runner_result->succ_case_list[runner_result->succ_case_count++], MAX_STR_LEN, "%s.%s",
                    test_suite->name, case_list[i]->name);
        }
        else
        {
            suite_result->fail_case_count++;
            snprintf(runner_result->fail_case_list[runner_result->fail_case_count++], MAX_STR_LEN, "%s.%s",
                    test_suite->name, case_list[i]->name);
        }

        suite_result->succ_assertion_count += case_result->succ_assertion_count;
        suite_result->fail_assertion_count += case_result->fail_assertion_count;
        suite_result->assertion_count += case_result->assertion_count;
        suite_result->time += case_result->time;
    }
}

static void calc_runner_suite_final_result(runner_result_t *runner_result, const test_suite_t *test_suite)
{
    suite_result_t *suite_result = test_suite->result;
    reset_suite_result(suite_result);
    suite_result->case_count = test_suite->case_count;
    runner_result->case_count += suite_result->case_count;

    calc_runner_suite_case_final_result(runner_result, test_suite);

    if (!suite_result->accessed)
    {
        snprintf(runner_result->skip_suite_list[runner_result->skip_suite_count++],
                MAX_STR_LEN, "%s", test_suite->name);
        return;
    }

    if (suite_result->is_filtered_out)
    {
        snprintf(runner_result->filtered_out_suite_list[runner_result->filtered_out_suite_count++],
                MAX_STR_LEN, "%s", test_suite->name);
        return;
    }

    if (suite_result->passed)
        snprintf(runner_result->succ_suite_list[runner_result->succ_suite_count++],
                MAX_STR_LEN, "%s", test_suite->name);
    else
        snprintf(runner_result->fail_suite_list[runner_result->fail_suite_count++],
                MAX_STR_LEN, "%s", test_suite->name);

    runner_result->succ_assertion_count += suite_result->succ_assertion_count;
    runner_result->fail_assertion_count += suite_result->fail_assertion_count;
    runner_result->assertion_count += suite_result->assertion_count;
    runner_result->time += suite_result->time;
}

static void calc_ut_result(const test_runner_t *test_runner)
{
    runner_result_t *result = test_runner->result;
    if (!result->accessed)
        return;

    reset_runner_result(result);
    result->suite_count = test_runner->suite_count;

    int i;
    test_suite_t** suite_list = test_runner->suite_list;
    for (i = 0; i < test_runner->suite_count; i++)
        calc_runner_suite_final_result(result, suite_list[i]);
}

void save_assertion_info(case_result_t *result, const char* file, int line, const char* expected, const char* actual,
                         const char* msg, ...)
{
    result->file = file;
    result->line = line;
    snprintf(result->expected, sizeof(result->expected), "%s", expected);
    snprintf(result->actual, sizeof(result->actual), "%s", actual);

    va_list args;
    va_start(args, msg);
    vsnprintf(result->user_msg, sizeof(result->user_msg), msg, args);
    va_end(args);
}

bool ut_init(int argc, char* argv[])
{
    _is_ut_init_called_ = true;
    _is_ut_init_successed_ = false;

    if (!init_ut_flag(argc, argv))
        return false;

    if (!init_test_runner(argv[0], &_test_runner_))
        return false;

    _is_ut_init_successed_ = true;
    return true;
}

bool ut_run(void)
{
    if (!_is_ut_init_called_)
    {
        print_ut_init_no_called_error();
        return false;
    }
    if (!_is_ut_init_successed_)
    {
        print_ut_init_error();
        return false;
    }

    if (!should_run())
        return true;

    int i;
    for (i = 0; i < UT_FLAG(repeat); i++)
    {
        bool ret = true;
        if (!run_test_runner(&_test_runner_))
            ret = false;

        calc_ut_result(&_test_runner_);
        print_ut_result(&_test_runner_);
        if (UT_FLAG(xml))
        {
            if (!print_ut_result_to_xml(&_test_runner_, i))
                ret = false;
        }

        if (!ret && !UT_FLAG(keep_going))
            return false;
    }

    return _test_runner_.result->passed;
}

void ut_fini(void)
{
    test_suite_t** suite_list = _test_runner_.suite_list;
    int i;
    for (i = 0; i < _test_runner_.suite_count; i++)
        free(suite_list[i]->case_list);

    free(suite_list);

    runner_result_t *runner_result = _test_runner_.result;
    free(runner_result->succ_suite_list);
    free(runner_result->fail_suite_list);
    free(runner_result->skip_suite_list);
    free(runner_result->filtered_out_suite_list);
    free(runner_result->succ_case_list);
    free(runner_result->fail_case_list);
    free(runner_result->skip_case_list);
    free(runner_result->filtered_out_case_list);
}
