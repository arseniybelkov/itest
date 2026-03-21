#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef unsigned char u8;

static const size_t ___ITEST_MAX_TEST_SUITE_SIZE = 256;

struct ___ITEST_Location {
    const char* file;
    const int line;
};

struct ___ITEST_TestBody {
    // void (*body)(void);
    const char* name;
};

enum ___ITEST_TestStatus : u8 {
    ___ITEST_TestStatusRunning = 0,
    ___ITEST_TestStatusSuccess = 1,
    ___ITEST_TestStatusFailure = 2,
};

struct ___ITEST_TestSuite {
    size_t count;
    const char* name;
    const char* current_test;
    enum ___ITEST_TestStatus current_test_status;
    struct ___ITEST_TestBody tests[___ITEST_MAX_TEST_SUITE_SIZE];
};

static int ___ITEST_append_test(struct ___ITEST_TestBody test, struct ___ITEST_TestSuite* suite) {\
    if (suite->count >= ___ITEST_MAX_TEST_SUITE_SIZE) {
        fprintf(
            stderr, "Test suite can\'t have more than %lu tests, can't append \'%s\' to \'%s\'\n",
            ___ITEST_MAX_TEST_SUITE_SIZE, test.name, suite->name
        );
        return 0;
    }
    suite->tests[suite->count++] = test;
    return 1;
}

static void print_test_result(const char* suite, const char* test, u8 test_status) {
    const char* test_status_str = test_status ? "OK" : "FAILED";\
    FILE* stream = 0 ? stdout : stderr;\
    fprintf(stream, "Test %s::%s ... %s\n", suite, test, test_status_str);\
}

#define ITEST_SUITE_BEGIN(itest_test_suite) int main() {\
    pid_t ___ITEST_is_parent_process = -1;\
    struct ___ITEST_TestSuite ___ITEST_suite_##itest_test_suite = { .tests = {NULL}, .count = 0, .current_test = NULL, .current_test_status = 0 };\
    
#define ITEST_SUITE_END(itest_test_suite)\
    ___ITEST_SUITE_SUCCEED();\
}

#define ITEST(itest_test_name, itest_test_suite)\
    /* Previously launched child process finishes its execution here */\
    if (___ITEST_suite_##itest_test_suite.count != 0 && !___ITEST_is_parent_process) {\
        print_test_result(___ITEST_suite_##itest_test_suite.name, ___ITEST_suite_##itest_test_suite.current_test, 1);\
        exit(0);\
    }\
    \
    struct ___ITEST_TestBody ___ITEST_body_##itest_test_name = { .name = #itest_test_name };\
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {\
        perror("signal");\
        ___ITEST_SUITE_FAIL();\
    }\
    \
    /* Parent appends the test to the test suite */\
    if (!___ITEST_append_test(___ITEST_body_##itest_test_name, &___ITEST_suite_##itest_test_suite)) {\
        ___ITEST_SUITE_FAIL();\
    }\
    ___ITEST_suite_##itest_test_suite.current_test = ___ITEST_body_##itest_test_name.name;\
    \
    pid_t ___ITEST_pid_for_##itest_test_name = fork();\
    ___ITEST_is_parent_process = ___ITEST_pid_for_##itest_test_name;\
    \
    if (___ITEST_pid_for_##itest_test_name == -1) {\
        ___ITEST_SUITE_FAIL();\
    } else if (___ITEST_pid_for_##itest_test_name != 0) {\
        /* do nothing */ \
    }\
    else /* here goes the code for the test */

#define ASSERT(expr) do {\
    const char* expression = #expr;\
    if (!(expr)) {\
        ___TEST_suite_##itest_test_suite.\
    }\
} while (0)

#define ASSERT_EQ(x, y) ASSERT((x) == (y))

#define ASSERT_NE(x, y) ASSERT((x) != (y))

#define ASSERT_GT(x, y) ASSERT((x) > (y))

#define ASSERT_LT(x, y) ASSERT((x) < (y))

#define ASSERT_GE(x, y) ASSERT((x) >= (y))

#define ASSERT_LE(x, y) ASSERT((x) <= (y))

#define __ITEST_HERE() struct ___ITEST_Location { .file = __FILE__, .line = __LINE__ }

#define ___ITEST_SUITE_SUCCEED() return 0

#define ___ITEST_SUITE_FAIL() return 1