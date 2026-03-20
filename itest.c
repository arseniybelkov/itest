#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static const size_t ___ITEST_MAX_TEST_SUITE_SIZE = 256;

struct ___ITEST_TestBody {
    // void (*body)(void);
    const char* name;
};

struct ___ITEST_TestSuite {
    size_t count;
    const char* name;
    struct ___ITEST_TestBody tests[___ITEST_MAX_TEST_SUITE_SIZE];
};

enum ___ITEST_Result : unsigned char {
    ___ITEST_ResultOk = 1,
    ___ITEST_ResultErr = 0,
};

static enum ___ITEST_Result ___ITEST_append_test(struct ___ITEST_TestBody test, struct ___ITEST_TestSuite* suite) {\
    if (suite->count >= ___ITEST_MAX_TEST_SUITE_SIZE) {
        fprintf(
            stderr, "Test suite can\'t have more than %lu tests, can't append \'%s\' to \'%s\'\n",
            ___ITEST_MAX_TEST_SUITE_SIZE, test.name, suite->name
        );
        return ___ITEST_ResultErr;
    }
    suite->tests[suite->count++] = test;
    return ___ITEST_ResultOk;
}


#define ITEST_SUITE_BEGIN(itest_test_suite) int main() {\
    pid_t prev_pid;\
    struct ___ITEST_TestSuite ___suite_##itest_test_suite = { .tests = {NULL}, .count = 0 };\
    
    
#define ITEST_SUITE_END(itest_test_suite)\
    ___ITEST_SUITE_SUCCEED();\
}

#define ITEST(itest_test_name, itest_test_suite)\
    /* Previously launched child process finishes its execution here */\
    if (___suite_##itest_test_suite.count != 0 && prev_pid == 0) {\
        exit(0);\
    }\
    struct ___ITEST_TestBody ___body_##itest_test_name = { .name = #itest_test_name };\
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {\
        perror("signal");\
        ___ITEST_SUITE_FAIL();\
    }\
    \
    pid_t ___ITEST_pid_for_##itest_test_name = fork();\
    prev_pid = ___ITEST_pid_for_##itest_test_name;\
    \
    if (___ITEST_pid_for_##itest_test_name == -1) {\
        ___ITEST_SUITE_FAIL();\
    } else if (___ITEST_pid_for_##itest_test_name != 0) {\
        /* Parent appends the test to the test suite */\
        if (!___ITEST_append_test(___body_##itest_test_name, &___suite_##itest_test_suite)) {\
            ___ITEST_SUITE_FAIL();\
        }\
    }\
    else


#define ___ITEST_SUITE_SUCCEED() return 0
#define ___ITEST_SUITE_FAIL() return 1