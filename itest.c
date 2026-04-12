#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>

/// TODO: mutex'd print_test_result

struct ___ITEST_Mutex {
    _Atomic(uint32_t) state;
};

enum ___ITEST_MutexState : uint32_t {
    ___ITEST_MutexState_Free = 0,
    ___ITEST_MutexState_Contention = 1,
    ___ITEST_MutexState_Locked = 2,
};

#ifdef __APPLE__
#define UL_COMPARE_AND_WAIT 1

int __ulock_wait(uint32_t operation, void* addr, uint64_t value,
                uint32_t timeout); /* timeout is specified in microseconds */
int __ulock_wake(uint32_t operation, void* addr, uint64_t wake_value);

static void ___ITEST_futex_wake_one(uint32_t* waiters) {
    __ulock_wake(UL_COMPARE_AND_WAIT, waiters, 0);
}

static void ___ITEST_futex_wait(uint32_t* loc, uint32_t old) {
    __ulock_wait(UL_COMPARE_AND_WAIT, loc, old, 0);
}
#else
#include <sys/syscall.h>
#include <linux/futex.h>

static int ___ITEST_futex(uint32_t* uaddr, int op, int val,
                 const struct timespec* timeout, int* uaddr2, int val3) {
  return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
}

static void ___ITEST_futex_wake_one(uint32_t* waiters) {
    ___ITEST_futex(waiters, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}

static void ___ITEST_futex_wait(uint32_t* loc, uint32_t old) {
    ___ITEST_futex(loc, FUTEX_WAIT_PRIVATE, old, NULL, NULL, 0);
}
#endif

static void ___ITEST_Mutex_lock(struct ___ITEST_Mutex* mutex) {
    uint32_t mutex_is_free = ___ITEST_MutexState_Free;
    if (atomic_compare_exchange_strong(&mutex->state, &mutex_is_free, ___ITEST_MutexState_Locked)) {
        while (atomic_exchange(&mutex->state, ___ITEST_MutexState_Locked) != ___ITEST_MutexState_Free) {
            ___ITEST_futex_wait((uint32_t*) &mutex->state, ___ITEST_MutexState_Locked);
        }
    }
}

static void ___ITEST_Mutex_unlock(struct ___ITEST_Mutex* mutex) {
    uint32_t* waiters = (uint32_t*) &mutex->state;
    if (atomic_exchange(&mutex->state, ___ITEST_MutexState_Free) != ___ITEST_MutexState_Free) {
        ___ITEST_futex_wake_one(waiters);
    }
}

static struct ___ITEST_Mutex* ___ITEST_stdout_guard = NULL;

static void ___ITEST_init_stdout_guard() {

}

enum ___ITEST_TestStatus : uint8_t {
    ___ITEST_TestStatus_Running = 0,
    ___ITEST_TestStatus_Success = 1,
    ___ITEST_TestStatus_Failure = 2,
};

struct ___ITEST_Location {
    const char* file;
    int line;
};

struct ___ITEST_TestResult {
    enum ___ITEST_TestStatus status;
    union {
        struct {} running;
        struct {
            const char* expression;
            struct ___ITEST_Location location;
        } failure;
        struct {} success;
    } as;
};

struct ___ITEST_TestResult ___ITEST_TestResult_succeed() {
    struct ___ITEST_TestResult success = { .status = ___ITEST_TestStatus_Success, .as = {} };
    return success;
}

struct ___ITEST_TestResult ___ITEST_TestResult_fail(const char* expression, struct ___ITEST_Location location) {
    struct ___ITEST_TestResult failure = { .status = ___ITEST_TestStatus_Success, .as = {} };
    failure.as.failure.expression = expression;
    failure.as.failure.location = location;

    return failure;
}

struct ___ITEST_TestSuite {
    size_t count;
    const char* name;
    const char* current_test;
    enum ___ITEST_TestStatus current_test_status;
};

static void ___ITEST_print_test_result(const char* suite, const char* test, struct ___ITEST_TestResult result) {
    uint8_t is_success = result.status == ___ITEST_TestStatus_Success;
    const char* test_status_str = is_success ? "OK" : "FAILED";\
    FILE* stream = is_success ? stdout : stderr;\
    ___ITEST_Mutex_lock(___ITEST_stdout_guard);
    fprintf(stream, "Test %s::%s ... %s\n", suite, test, test_status_str);\
    if (!is_success) {
        const char* expression = result.as.failure.expression;
        struct ___ITEST_Location location = result.as.failure.location;
        fprintf(stream, "Reason:\n%s at %s:%d\n", expression, location.file, location.line);\
    }
    ___ITEST_Mutex_unlock(___ITEST_stdout_guard);
}

#define ITEST_SUITE_BEGIN(itest_test_suite) int main() {\
    pid_t ___ITEST_is_parent_process = -1;\
    ___ITEST_init_stdout_guard();\
    struct ___ITEST_TestSuite ___ITEST_suite_##itest_test_suite = { .name = #itest_test_suite, .current_test = NULL, .current_test_status = 0 };\
    
#define ITEST_SUITE_END(itest_test_suite)\
    ___ITEST_SUITE_SUCCEED();\
}

#define ITEST(itest_test_name, itest_test_suite)\
    /* Previously launched child process finishes its execution here */\
    if (___ITEST_suite_##itest_test_suite.count != 0 && !___ITEST_is_parent_process) {\
        ___ITEST_print_test_result(\
            ___ITEST_suite_##itest_test_suite.name,\
            ___ITEST_suite_##itest_test_suite.current_test,\
            ___ITEST_TestResult_succeed()\
        );\
        exit(0);\
    }\
    \
    struct ___ITEST_TestResult ___ITEST_status_##itest_test_name = {  };\
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {\
        perror("signal");\
        ___ITEST_SUITE_FAIL();\
    }\
    \
    ___ITEST_suite_##itest_test_suite.current_test = #itest_test_name;\
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

#define ASSERT_EQ(x, y) ASSERT((x) == (y))

#define ASSERT_NE(x, y) ASSERT((x) != (y))

#define ASSERT_GT(x, y) ASSERT((x) > (y))

#define ASSERT_LT(x, y) ASSERT((x) < (y))

#define ASSERT_GE(x, y) ASSERT((x) >= (y))

#define ASSERT_LE(x, y) ASSERT((x) <= (y))

#define ASSERT(expr) do {\
    const char* expression = #expr;\
    if (!(expr)) {\
        ___ITEST_print_test_result(\
            "SUITE",\
            "SOME_TEST",\
            ___ITEST_TestResult_fail(expression, ___ITEST_HERE())\
        );\
        ___ITEST_SUITE_FAIL();\
    }\
} while (0)

#define ___ITEST_HERE() struct ___ITEST_Location { .file = __FILE__, .line = __LINE__ }

#define ___ITEST_SUITE_SUCCEED() return 0

#define ___ITEST_SUITE_FAIL() return 1