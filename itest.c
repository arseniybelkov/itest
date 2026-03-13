#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

static int ___ITEST_append_test(struct ___ITEST_TestBody test, struct ___ITEST_TestSuite* suite) {\
    if (suite->count >= ___ITEST_MAX_TEST_SUITE_SIZE) {
        fprintf(
            stderr, "Test suite can\'t have more than %lu tests, can't append \'%s\' to \'%s\'\n",
            ___ITEST_MAX_TEST_SUITE_SIZE, test.name, suite->name
        );
        return 1;
    }
    suite->tests[suite->count++] = test;
    return 0;
}

// void ___ITEST_run_all_tests(struct ___ITEST_TestSuite* suite) {
//     for (size_t idx = 0; idx != suite->count; ++idx) {
//         suite->tests[idx].body();
//     }
//     suite->count = 0;
// }

#define ITEST_SUITE_BEGIN(itest_test_suite) int main() {\
    struct ___ITEST_TestSuite ___suite_##itest_test_suite = { .tests = {NULL}, .count = 0 };\


#define ITEST_SUITE_END(itest_test_suite)\
    return 0;\
}

#define ITEST(itest_test_name, itest_test_suite)\
    struct ___ITEST_TestBody ___body_##itest_test_name = { .name = #itest_test_name };\
    /*if (___ITEST_append_test(___body_##itest_test_name, &___suite_##itest_test_suite)) {\
        return 1;\
    }\
    */\
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {\
        perror("signal");\
        exit(EXIT_FAILURE);\
    }\
    pid_t ___ITEST_pid_for_##itest_test_name = fork();\
    if (___ITEST_pid_for_##itest_test_name == -1) {\
        exit(EXIT_FAILURE);\
    } else if (___ITEST_pid_for_##itest_test_name != 0) {\
    }\
    else
