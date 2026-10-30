#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/mman.h>

/*
    Test Results & Statuses
*/

enum ___ITEST_test_status : uint8_t {
    ___ITEST_test_status_running = 0,
    ___ITEST_test_status_success = 1,
    ___ITEST_test_status_failure = 2,
};

struct ___ITEST_location {
    const char* file;
    int line;
};

struct ___ITEST_test_result {
    enum ___ITEST_test_status status;
    union {
        struct {} running;
        struct {
            const char* expression;
            struct ___ITEST_location location;
        } failure;
        struct {} success;
    } as;
};

inline static struct ___ITEST_test_result ___ITEST_test_succeed() {
    struct ___ITEST_test_result success = { .status = ___ITEST_test_status_success, .as = {} };
    return success;
}

inline static struct ___ITEST_test_result ___ITEST_test_failed(const char* expression, const struct ___ITEST_location location) {
    struct ___ITEST_test_result failure = { .status = ___ITEST_test_status_failure, .as = {} };
    failure.as.failure.expression = expression;
    failure.as.failure.location = location;
    return failure;
}

struct ___ITEST_test_suite {
    size_t count;
    const char* name;
    const char* current_test;
    enum ___ITEST_test_status current_test_status;
};

static const char* ___ITEST_NO_COLOR = "\x1B[0m";
static const char* ___ITEST_RED_COLOR = "\x1B[31m";
static const char* ___ITEST_GREEN_COLOR = "\x1B[32m";

static void ___ITEST_print_test_result(
    const char* suite, const char* test,
    const struct ___ITEST_test_result result
) {
    uint8_t is_success = result.status == ___ITEST_test_status_success;
    const char* test_status_str = is_success ? "OK" : "FAILED";
    FILE* stream = is_success ? stdout : stderr;

    #ifdef ITEST_NO_COLOR_IO
    fprintf(stream, "Test %s::%s ... %s\n", suite, test, test_status_str);
    #else
    const char* color = is_success ? ___ITEST_GREEN_COLOR : ___ITEST_RED_COLOR;
    fprintf(stream, "%sTest %s::%s ... %s%s\n", ___ITEST_NO_COLOR, suite, test, color, test_status_str);
    #endif
    if (!is_success) {
        const char* expression = result.as.failure.expression;
        struct ___ITEST_location location = result.as.failure.location;
        fprintf(stream, "Reason:\n%s at %s:%d\n", expression, location.file, location.line);
    }
}

static struct ___ITEST_test_suite ___ITEST_test_suite = {
    .count = 0, .name = NULL, .current_test = NULL, .current_test_status = ___ITEST_test_status_running
};

#define ITEST_SUITE_BEGIN(itest_test_suite) int main() {\
    ___ITEST_test_suite.name = #itest_test_suite;\
    
#define ITEST_SUITE_END()\
    ___ITEST_SUITE_SUCCEED();\
}

#define ITEST_SINGLE_WORKER(itest_test_name)\
    if (___ITEST_test_suite.count != 0) {\
        ___ITEST_print_test_result(\
            ___ITEST_test_suite.name,\
            ___ITEST_test_suite.current_test,\
            ___ITEST_test_succeed()\
        );\
        /* ___ITEST_SUITE_SUCCEED(); */ \
    }\
    \
    struct ___ITEST_test_result ___ITEST_status_##itest_test_name = { };\
    ___ITEST_test_suite.current_test = #itest_test_name;\
    ___ITEST_test_suite.count++;\
    /* Test code goes here */\


#define ITEST(itest_test_name)\
    /* Previously launched child process finishes its execution here */\
    if (___ITEST_test_suite.count != 0 && ___ITEST_is_child_process) {\
        ___ITEST_print_test_result(\
            ___ITEST_test_suite.name,\
            ___ITEST_test_suite.current_test,\
            ___ITEST_test_succeed()\
        );\
        ___ITEST_SUITE_SUCCEED();\
    }\
    \
    struct ___ITEST_test_result ___ITEST_status_##itest_test_name = {  };\
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {\
        perror("signal");\
        ___ITEST_SUITE_FAIL();\
    }\
    \
    ___ITEST_test_suite.current_test = #itest_test_name;\
    \
    pid_t ___ITEST_pid_for_##itest_test_name = fork();\
    ___ITEST_test_suite.count += 1;\
    ___ITEST_is_child_process = ___ITEST_pid_for_##itest_test_name == 0;\
    \
    if (___ITEST_pid_for_##itest_test_name == -1) {\
        ___ITEST_SUITE_FAIL();\
    } else if (___ITEST_pid_for_##itest_test_name != 0) {\
        /* do nothing */ \
    }\
    else /* here goes the code for the test */

#define ASSERT_EQ(x, y) ASSERT((x) == (y))

#define ASSERT_NE(x, y) ASSERT((x) != (y))

#define ASSERT_GT(x, y) ASSERT((x) > (y))

#define ASSERT_LT(x, y) ASSERT((x) < (y))

#define ASSERT_GE(x, y) ASSERT((x) >= (y))

#define ASSERT_LE(x, y) ASSERT((x) <= (y))

#define ASSERT(expr) do {\
    const char* expression = #expr;\
    struct ___ITEST_location location = ___ITEST_HERE();\
    ___ITEST_assert_impl((expr), expression, location, &___ITEST_test_suite);\
} while (0)

inline static void ___ITEST_assert_impl(
    const uint8_t expr_success, const char* expression,
    struct ___ITEST_location loc, const struct ___ITEST_test_suite* suite
) {
    const uint8_t is_inside_a_test = suite->name != NULL;
    if (!expr_success) {
        struct ___ITEST_test_result res = ___ITEST_test_failed(expression, loc);
        if (is_inside_a_test) {
            ___ITEST_print_test_result(suite->name, suite->current_test, res);
        } else {
            #ifdef ITEST_NO_COLOR_IO
            fprintf(stderr, "Assert failed: %s at %s:%d\n", expression, loc.file, loc.line);
            #else
            fprintf(stderr, "%sAssert failed: %s at %s:%d\n", ___ITEST_RED_COLOR, expression, loc.file, loc.line);
            #endif
        }
        exit(1);
    }
}

#define ___ITEST_HERE() { .file = __FILE__, .line = __LINE__ }

#define ___ITEST_SUITE_SUCCEED() return 0

#define ___ITEST_SUITE_FAIL() return 1
