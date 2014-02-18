#include "zcut.h"

#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>

#define COLOR_FORMAT        "\033[%d;3%dm"
#define UNDERLINE_FORMAT    "\033[4m"
#define RESET_COLOR_FORMAT  "\033[0m"

bool UT_FLAG(no_color);
bool UT_FLAG(no_filtered_out_result);
bool UT_FLAG(highlight);
char UT_FLAG(xml_path)[MAX_STR_LEN];

static char* const COLOR_TERM_LIST[] =
{
    "cygwin",
    "linux",
    "screen",
    "xterm",
    "xterm-color",
    "xterm-256color"
};
static const unsigned int COLOR_TERM_LIST_COUNT = sizeof(COLOR_TERM_LIST) / sizeof(COLOR_TERM_LIST[0]);

static const char BORDER        = '|';
static char* UT_LABEL           = "     UT     ";
static char* RUNNER_LABEL       = "   RUNNER   ";
static char* SUITE_LABEL        = "   SUITE    ";
static char* CASE_LABEL         = "    CASE    ";
static char* SETUP_LABEL        = "   SETUP    ";
static char* TEARDOWN_LABEL     = "  TEARDOWN  ";
static char* INTERNAL_ERR_LABEL = "INTERNAL ERR";
static char* SKIPED_LABEL       = "   SKIPED   ";
static char* FILTERED_OUT_LABEL = "FILTERED OUT";
static char* BLANK_LABEL        = "            ";
static char* FILE_LINE_LABEL    = " file: line ";
static char* EXPECTED_LABEL     = "  expected  ";
static char* ACTUAL_LABEL       = "   actual   ";
static char* USER_MESSAGE_LABEL = "user message";
static char* PASSED_LABEL       = "   PASSED   ";
static char* FAILED_LABEL       = "   FAILED   ";
static char* TIME_LABEL         = "    TIME    ";
static char* RUNNER_NAME        = "Runner";
static char* SUITE_NAME         = "Suite";
static char* CASE_NAME          = "Case";
static char* SKIPPED            = "SKIPPED";
static char* FILTERED_OUT       = "FILTERED_OUT";
static char* PASSED             = "PASSED";
static char* FAILED             = "FAILED";

static char** const LABEL_LIST[] =
{
    &RUNNER_LABEL,
    &SUITE_LABEL,
    &CASE_LABEL,
    &SETUP_LABEL,
    &TEARDOWN_LABEL
};
static char** const TEST_TYPE_NAME_LIST[] =
{
    &RUNNER_NAME,
    &SUITE_NAME,
    &CASE_NAME
};

static char* ZCUT       = "zCUT";
static char* VERSION    = "0";
static char* HELP = \
"Usage: test_bin [-bC] [-fF FILTER_EXPRESSION] [-Hkl] [-r REPEAT_COUNT] [-s] [-x [XML_PATH]] [-hv]\n"
"  -b, --break-on-failure           Exit unit test when a assertion failed.\n"
"  -C, --no-color                   Disabled colored output. Default is enabled.\n"
"  -f, --case-filter                Choose test case to run with simple regular expression.\n"
"  -F, --suite-filter               Choose test suite to run with simple regular expression.\n"
"  -H, --highlight                  Enable highlighted output. Default is disabled.\n"
"  -k, --keep-going                 When repeat count is larger than 0, keep unit test going when error occur.\n"
"  -l, --list                       Just list out all the test suite and test case instead of running them.\n"
"  -r, --repeat                     Run unit test for a repeat count, in range [0, INT_MAX].\n"
"  -R, --no-filtered-out-result     Do not output filterd out case or suite result.\n"
"  -s, --shuffle                    Randomize the order of test suite and test case.\n"
"  -x, --xml-path                   Generate an XML report with detail informaion of the unit test.\n"
"  -h, --help                       Display this help and exit.\n"
"  -v, --version                    Display version and exit.\n";

static const int INDENT = 2;

static bool should_use_color(void)
{
    if (UT_FLAG(no_color))
        return false;

    if (isatty(fileno(stdout)) == 0)
        return false;

    const char* term = getenv("TERM");
    if (term == NULL)
        return false;

    unsigned int i;
    for (i = 0; i < COLOR_TERM_LIST_COUNT; i++)
    {
        if (strcmp(term, COLOR_TERM_LIST[i]) == 0)
            return true;
    }

    return false;
}

static void color_print(color_t color, const char* string)
{
    if (!should_use_color())
    {
        printf("%s ", string);
        return;
    }

    printf(COLOR_FORMAT "%s " RESET_COLOR_FORMAT, UT_FLAG(highlight), color, string);
    fflush(stdout);
}

static void color_underline_print(color_t color, const char* string, char placeholder)
{
    if (!should_use_color())
    {
        printf("%c%s%c ", placeholder, string, placeholder);
        return;
    }

    printf(COLOR_FORMAT "%c" UNDERLINE_FORMAT "%s" RESET_COLOR_FORMAT COLOR_FORMAT "%c " RESET_COLOR_FORMAT,
            UT_FLAG(highlight), color, placeholder, string, UT_FLAG(highlight), color, placeholder);
    fflush(stdout);
}

static void print_label(color_t color, const char* label)
{
    char string[MAX_STR_LEN];
    snprintf(string, MAX_STR_LEN, "%c%s%c", BORDER, label, BORDER);
    color_print(color, string);
}

static void print_underline_label(color_t color, const char* string)
{
    color_underline_print(color, string, BORDER);
}

static void print_underline_blank(color_t color)
{
    color_underline_print(color, BLANK_LABEL, ' ');
    printf("\n");
}

void print_help(void)
{
    printf("%s", HELP);
}

void print_version(void)
{
    printf("%s %s\n", ZCUT, VERSION);
}

static const char* get_label(test_type_t test_type)
{
    return *LABEL_LIST[(int)test_type];
}

static const char* get_test_type_name(test_type_t test_type)
{
    return *TEST_TYPE_NAME_LIST[(int)test_type];
}

static void print_begin_label(test_type_t test_type, const char* msg)
{
    const char* label = get_label(test_type);
    if (test_type == RUNNER || test_type == SUITE)
        print_underline_label(GREEN, label);
    else
        print_label(GREEN, label);

    printf("%s\n", msg);
}

static void print_end_label(bool passed, const char* msg)
{
    color_t color = passed ? GREEN : RED;
    const char* label = passed ? PASSED_LABEL : FAILED_LABEL;
    print_underline_label(color, label);
    printf("%s\n", msg);
}

void print_runner_begin(const test_runner_t *test_runner)
{
    print_underline_blank(GREEN);

    char msg[MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "Test runner \"%s\" begin", test_runner->name);
    print_begin_label(RUNNER, msg);
}

void print_runner_end(const test_runner_t *test_runner, setup_teardown_func_t teardown)
{
    const runner_result_t *result = test_runner->result;
    if (!teardown)
    {
        color_t color = result->passed ? GREEN : RED;
        print_underline_blank(color);
    }

    char msg[MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "Test runner \"%s\" end [(%d suite) (%d case) (%d assertsion) (%d ms)]",
            test_runner->name, result->suite_count, result->case_count, result->assertion_count, result->time);
    print_end_label(result->passed, msg);
}

void print_suite_begin(const test_suite_t *test_suite)
{
    print_underline_blank(GREEN);

    char msg[MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "Test suite \"%s\" begin", test_suite->name);
    print_begin_label(SUITE, msg);
}

void print_suite_end(const test_suite_t *test_suite)
{
    const suite_result_t *result = test_suite->result;
    char msg[MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "Test suite \"%s\" end [(%d case) (%d assertion) (%d ms)]",
            test_suite->name, result->case_count, result->assertion_count, result->time);
    print_end_label(result->passed, msg);
}

void print_case_begin(const test_case_t *test_case)
{
    print_begin_label(CASE, test_case->name);
}

void print_case_end(const test_case_t *test_case)
{
    const case_result_t *result = test_case->result;
    char msg[MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "%s [(%d assertion) (%d ms)]",
            test_case->name, result->assertion_count, result->time);
    print_end_label(result->passed, msg);
}

static void print_setup_teardown_begin_label(test_type_t setup_teardown, const char* msg)
{
    const char* label = get_label(setup_teardown);
    print_label(GREEN, label);
    printf("%s", msg);
}

static void print_setup_teardown_end_label(bool passed, const char* msg)
{
    color_t color = passed ? GREEN : RED;
    const char* label = passed ? PASSED_LABEL : FAILED_LABEL;
    print_underline_label(color, label);
    printf("%s", msg);
}

void print_setup_begin(test_type_t test_type)
{
    char msg[MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "%s setup begin\n", get_test_type_name(test_type));
    print_setup_teardown_begin_label(SETUP, msg);
}

void print_setup_end(test_type_t test_type, bool passed, int time)
{
    char msg[MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "%s setup end [(%d ms)]\n", get_test_type_name(test_type), time);
    print_setup_teardown_end_label(passed, msg);
}

void print_teardown_begin(test_type_t test_type)
{
    if (test_type == RUNNER)
        print_underline_blank(GREEN);

    char msg[MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "%s teardown begin\n", get_test_type_name(test_type));
    print_setup_teardown_begin_label(TEARDOWN, msg);
}

void print_teardown_end(test_type_t test_type, bool passed, int time)
{
    char msg[MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "%s teardown end [(%d ms)]\n", get_test_type_name(test_type), time);
    print_setup_teardown_end_label(passed, msg);
}

void print_assertion_info(const char* file, int line, const char* expected, const char* actual, const char* msg, ...)
{
    print_label(CYAN, FILE_LINE_LABEL);
    printf("%s: %d\n", file, line);

    if (strlen(expected) > 0)
    {
        print_label(CYAN, EXPECTED_LABEL);
        printf("%s\n", expected);
    }

    if (strlen(actual) > 0)
    {
        print_label(CYAN, ACTUAL_LABEL);
        printf("%s\n", actual);
    }

    char formatted_msg[MAX_STR_LEN];
    va_list args;
    va_start(args, msg);
    vsnprintf(formatted_msg, sizeof(formatted_msg), msg, args);
    va_end(args);
    size_t len = strlen(formatted_msg);
    if (len > 0)
    {
        print_label(CYAN, USER_MESSAGE_LABEL);
        printf("%s", formatted_msg);
        if (formatted_msg[len - 1] != '\n')
            printf("\n");
    }

    fflush(stdout);
}

void print_ut_list(const test_runner_t *test_runner)
{
    printf("UT: %s\n", test_runner->name);

    int suite_index;
    for (suite_index = 0; suite_index < test_runner->suite_count; suite_index++)
    {
        test_suite_t *test_suite = test_runner->suite_list[suite_index];
        printf("%s\n", test_suite->name);

        int case_index;
        for (case_index = 0; case_index < test_suite->case_count; case_index++)
            printf("%*c%s\n", INDENT, ' ', test_suite->case_list[case_index]->name);
    }
}

static void print_case_result(const runner_result_t *result)
{
    print_underline_blank(GREEN);
    print_label(GREEN, CASE_LABEL);
    printf("%d\n", result->case_count);

    print_label(GREEN, PASSED_LABEL);
    printf("%d\n", result->succ_case_count);
    int i;
    for (i = 0; i < result->succ_case_count; i++)
    {
        print_label(GREEN, BLANK_LABEL);
        printf("%s\n", result->succ_case_list[i]);
    }

    if (result->fail_case_count > 0)
        print_label(RED, FAILED_LABEL);
    else
        print_label(GREEN, FAILED_LABEL);
    printf("%d\n", result->fail_case_count);
    for (i = 0; i < result->fail_case_count; i++)
    {
        print_label(RED, BLANK_LABEL);
        printf("%s\n", result->fail_case_list[i]);
    }

    if (result->skip_case_count > 0)
    {
        print_label(BLUE, SKIPED_LABEL);
        printf("%d\n", result->skip_case_count);
        for (i = 0; i < result->skip_case_count; i++)
        {
            print_label(BLUE, BLANK_LABEL);
            printf("%s\n", result->skip_case_list[i]);
        }
    }

    if (UT_FLAG(no_filtered_out_result) || result->filtered_out_case_count == 0)
        return;

    if (result->filtered_out_case_count > 0)
    {
        print_label(YELLOW, FILTERED_OUT_LABEL);
        printf("%d\n", result->filtered_out_case_count);
        for (i = 0; i < result->filtered_out_case_count; i++)
        {
            print_label(YELLOW, BLANK_LABEL);
            printf("%s\n", result->filtered_out_case_list[i]);
        }
    }
}

static void print_suite_result(const runner_result_t *result)
{
    color_underline_print(GREEN, BLANK_LABEL, BORDER);
    printf("\n");
    print_label(GREEN, SUITE_LABEL);
    printf("%d\n", result->suite_count);

    print_label(GREEN, PASSED_LABEL);
    printf("%d\n", result->succ_suite_count);
    int i;
    for (i = 0; i < result->succ_suite_count; i++)
    {
        print_label(GREEN, BLANK_LABEL);
        printf("%s\n", result->succ_suite_list[i]);
    }

    if (result->fail_suite_count > 0)
        print_label(RED, FAILED_LABEL);
    else
        print_label(GREEN, FAILED_LABEL);
    printf("%d\n", result->fail_suite_count);
    for (i = 0; i < result->fail_suite_count; i++)
    {
        print_label(RED, BLANK_LABEL);
        printf("%s\n", result->fail_suite_list[i]);
    }

    if (result->skip_suite_count > 0)
    {
        print_label(BLUE, SKIPED_LABEL);
        printf("%d\n", result->skip_suite_count);
        for (i = 0; i < result->skip_suite_count; i++)
        {
            print_label(BLUE, BLANK_LABEL);
            printf("%s\n", result->skip_suite_list[i]);
        }
    }

    if (UT_FLAG(no_filtered_out_result) || result->filtered_out_suite_count == 0)
        return;

    if (result->filtered_out_suite_count > 0)
    {
        print_label(YELLOW, FILTERED_OUT_LABEL);
        printf("%d\n", result->filtered_out_suite_count);
        for (i = 0; i < result->filtered_out_suite_count; i++)
        {
            print_label(YELLOW, BLANK_LABEL);
            printf("%s\n", result->filtered_out_suite_list[i]);
        }
    }
}

static void print_runner_result(const test_runner_t *test_runner)
{
    color_underline_print(GREEN, BLANK_LABEL, BORDER);
    printf("\n");
    print_label(GREEN, UT_LABEL);
    printf("%s\n", test_runner->name);

    const runner_result_t *result = test_runner->result;
    if (!result->accessed)
    {
        print_label(RED, SKIPED_LABEL);
        printf("\n");
        return;
    }

    print_label(GREEN, TIME_LABEL);
    printf("%d ms\n", result->time);

    if (result->passed)
    {
        print_underline_label(GREEN, PASSED_LABEL);
        printf("\n");
    }
    else
    {
        print_underline_label(RED, FAILED_LABEL);
        printf("\n");
    }
}

void print_ut_result(const test_runner_t *test_runner)
{
    printf("\n");

    const runner_result_t *result = test_runner->result;
    if (result->accessed)
    {
        print_case_result(result);
        print_suite_result(result);
    }
    print_runner_result(test_runner);
}

static bool get_xml_path_default(const char* test_bin_name, int repeat, char xml_path[PATH_MAX])
{
    if (strlen(UT_FLAG(xml_path)) != 0)
        return false;

    if (repeat > 0)
        snprintf(xml_path, PATH_MAX, "%s_%d.xml", test_bin_name, repeat);
    else
        snprintf(xml_path, PATH_MAX, "%s.xml", test_bin_name);

    return true;
}

static bool get_xml_path_with_dir(const char* test_bin_name, int repeat, char xml_path[PATH_MAX])
{
    struct stat fs;
    if (stat(UT_FLAG(xml_path), &fs) == -1)
        return false;

    if (!S_ISDIR(fs.st_mode))
        return false;

    if (repeat > 0)
        snprintf(xml_path, PATH_MAX, "%s/%s_%d.xml", UT_FLAG(xml_path), test_bin_name, repeat);
    else
        snprintf(xml_path, PATH_MAX, "%s/%s.xml", UT_FLAG(xml_path), test_bin_name);

    return true;
}

static bool get_xml_path(const char* test_bin_name, int repeat, char xml_path[PATH_MAX])
{
    if (get_xml_path_default(test_bin_name, repeat, xml_path))
        return true;

    if (get_xml_path_with_dir(test_bin_name, repeat, xml_path))
        return true;

    snprintf(xml_path, PATH_MAX, "%s", UT_FLAG(xml_path));
    return true;
}

static const char* escape_xml(const char* string)
{
    static char escape_str[MAX_STR_LEN];

    char* index = escape_str;
    const char* replace_str = NULL;
    while (*string && (index - escape_str) < MAX_STR_LEN)
    {
        switch (*string)
        {
        case '<':
            replace_str = "&lt;";
            break;
        case '>':
            replace_str = "&gt;";
            break;
        case '&':
            replace_str = "&amp;";
            break;
        case '\'':
            replace_str = "&apos;";
            break;
        case '\"':
            replace_str = "&quot;";
            break;
        default:
            *index++ = *string++;
            continue;
        }

        while (*replace_str)
            *index++ = *replace_str++;
        string++;
    }
    *index = '\0';

    return escape_str;
}

static void write_test_case_result(FILE *xml, const test_case_t *test_case, int indent)
{
    const case_result_t *case_result = test_case->result;
    const char* result;
    if (!case_result->accessed)
        result = SKIPPED;
    else if (case_result->is_filtered_out)
        result = FILTERED_OUT;
    else if (case_result->passed)
        result = PASSED;
    else
        result = FAILED;

    if (!case_result->accessed
        || case_result->passed
        || (!UT_FLAG(no_filtered_out_result) && case_result->is_filtered_out))
    {
        fprintf(xml, "%*c<test_case name=\"%s\" result=\"%s\" assertion=\"%d\" time=\"%dms\"/>\n", indent, ' ',
                escape_xml(test_case->name), result, case_result->assertion_count, case_result->time);
    }
    else
    {
        fprintf(xml, "%*c<test_case name=\"%s\" result=\"%s\" assertion=\"%d\" time=\"%dms\">\n", indent, ' ',
                escape_xml(test_case->name), result, case_result->assertion_count, case_result->time);
        fprintf(xml, "%*c<message file=\"%s\" line=\"%d\" expected=\"%s\" actual=\"%s\" user_msg=\"%s\"/>\n",
                indent + INDENT, ' ',
                case_result->file, case_result->line, escape_xml(case_result->expected), escape_xml(case_result->actual),
                case_result->user_msg);
        fprintf(xml, "%*c</test_case>\n", indent, ' ');
    }
}

static void write_test_suite_result(FILE *xml, const test_suite_t *test_suite, int indent)
{
    const suite_result_t *suite_result = test_suite->result;
    const char* result;
    if (!suite_result->accessed)
        result = SKIPPED;
    else if (suite_result->is_filtered_out)
        result = FILTERED_OUT;
    else if (suite_result->passed)
        result = PASSED;
    else
        result = FAILED;

    if (UT_FLAG(no_filtered_out_result) && suite_result->is_filtered_out)
        return;

    fprintf(xml, "%*c<test_suite name=\"%s\" result=\"%s\" test_case=\"%d\" assertion=\"%d\" time=\"%dms\">\n",
            indent, ' ', escape_xml(test_suite->name),
            result, suite_result->case_count, suite_result->assertion_count, suite_result->time);

    if (suite_result->accessed && !suite_result->is_filtered_out)
    {
        int i;
        for (i = 0; i < test_suite->case_count; i++)
            write_test_case_result(xml, test_suite->case_list[i], indent + INDENT);
    }

    fprintf(xml, "%*c</test_suite>\n", indent, ' ');
}

static void write_test_runner_result(FILE *xml, const test_runner_t *test_runner)
{
    const runner_result_t *runner_result = test_runner->result;
    const char* result;
    if (!runner_result->accessed)
        result = SKIPPED;
    else if (runner_result->passed)
        result = PASSED;
    else
        result = FAILED;

    fprintf(xml, "<ut name=\"%s\" result=\"%s\" test_suite=\"%d\" test_case=\"%d\" assertion=\"%d\" time=\"%dms\">\n",
            escape_xml(test_runner->name), result, runner_result->suite_count, runner_result->case_count,
            runner_result->assertion_count, runner_result->time);

    if (runner_result->accessed)
    {
        int i;
        for (i = 0; i < test_runner->suite_count; i++)
            write_test_suite_result(xml, test_runner->suite_list[i], INDENT);
    }

    fprintf(xml, "</ut>\n");
}

static bool write_ut_result_to_xml(const test_runner_t *test_runner, const char* xml_path)
{
    FILE *xml = fopen(xml_path, "w+");
    if (xml == NULL)
    {
        fprintf(stderr, "fopen(%s, w+): %m\n", xml_path);
        return false;
    }

    fprintf(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    write_test_runner_result(xml, test_runner);

    fclose(xml);
    return true;
}

bool print_ut_result_to_xml(const test_runner_t *test_runner, int repeat)
{
    char xml_path[PATH_MAX];
    if (!get_xml_path(test_runner->test_bin_name, repeat, xml_path))
        return false;

    if (!write_ut_result_to_xml(test_runner, xml_path))
        return false;

    return true;
}

void print_ut_init_no_called_error(void)
{
    fprintf(stderr, "`ut_init(argc, argv)' must be called before `ut_run()'.\n");
}

void print_ut_init_error(void)
{
    fprintf(stderr, "`ut_init(argc, argv)' error.\n");
}

void print_ut_flag_int_type_warning(const char* flag, const char* value, int default_value)
{
    fprintf(stderr, "UT_ENV_FLAG `%s = %s' is invalid, use `%d' default.\n", flag, value, default_value);
}

void print_ut_flag_int_value_warning(const char* flag, int value, int min, int max, int default_value)
{
    fprintf(stderr, "UT_ENV_FLAG `%s = %d' is out of range [%d, %d], use `%d' default.\n",
            flag, value, min, max, default_value);
}

void print_ut_flag_int_type_error(const char* option, const char* value)
{
    fprintf(stderr, "UT_OPTION `%s = %s' is invalid.\n", option, value);
}

void print_ut_flag_int_value_error(const char* option, int value, int min, int max)
{
    fprintf(stderr, "UT_OPTION `%s = %d' is out of range [%d, %d].\n", option, value, min, max);
}

void print_non_option_error(int optind, int argc, char* argv[])
{
    fprintf(stderr, "non-option ARGV-elements: ");
    while (optind < argc)
        fprintf(stderr, "%s ", argv[optind++]);
    fprintf(stderr, "\n");
}

void print_error(const char* file, const char* function, int line, const char* msg, ...)
{
    char format[MAX_STR_LEN];
    snprintf(format, sizeof(format), "[%s, %s(), %d] %s", file, function, line, msg);

    char formatted_msg[MAX_STR_LEN];
    va_list args;
    va_start(args, msg);
    vsnprintf(formatted_msg, sizeof(formatted_msg), format, args);
    va_end(args);

    print_label(PURPLE, INTERNAL_ERR_LABEL);
    printf("%s\n", formatted_msg);
}
